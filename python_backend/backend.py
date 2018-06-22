import os, random, time, sys
import opentracing
import zipkin_ot
import flask

app = flask.Flask("calc_backend")

opentracing.tracer = zipkin_ot.Tracer(service_name="demo_arch",
        collector_host=os.environ.get("ZIPKIN_HOST"))

files_dir = None

@app.before_request
def launch_trace():
    headers = {}
    for k, v in flask.request.headers.items():
        headers[k] = v

    parent_context = opentracing.tracer.extract(opentracing.Format.HTTP_HEADERS, headers)
    span = opentracing.tracer.start_span(operation_name="calc_service",
            child_of=parent_context)

    span.set_tag("span.kind", "server")
    span.set_tag("component", "calc frontend service")
    span.set_tag("http.url", "http://calc_backend:4570")

    flask.g.span = span

@app.after_request
def stop_trace(response):
    flask.g.span.finish()

    return response

@app.route("/")
def index():
    batch_id = random.randint(0, 100)

    flask.g.span.set_tag("batch.id", batch_id)

    span_data = {}
    opentracing.tracer.inject(flask.g.span.context, opentracing.Format.HTTP_HEADERS, span_data)

    # Zipkin specific data extraction
    trace_id = span_data["x-b3-traceid"]
    span_id = span_data["x-b3-traceid"]
    parent_id = flask.g.span.parent_id # not provided by implementation: span_data["x-b3-parentspanid"]

    # preparing files for C++ service
    in_filename = ("{}_{}_{}_{:02x}.in".format(batch_id, trace_id, span_id, parent_id))

    flask.g.span.log_event("enqueuing", { "file": in_filename })
    open(os.path.join(files_dir, in_filename), "w").close()

    out_filename = ("{}_{}_{}_{:02x}.out".format(batch_id, trace_id, span_id, parent_id))

    attempts = 0

    # waiting for processing
    while True:
        files = os.listdir(files_dir)

        if out_filename in files:
            # everything Ok, cleanup files
            os.unlink(os.path.join(files_dir, out_filename))
            break
        else:
            attempts = attempts + 1
            flask.g.span.log_event("waiting", { "attempts": attempts })

            if 20 == attempts:
                flask.g.span.log_event("timeout", { "file": out_filename })
                return "{\"status\":\"failure\"}", 408

            time.sleep(0.001)

    flask.g.span.log_event("done", { "file": out_filename })

    return "{\"status\":\"ok\"}"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Please give the files directory!"
        sys.exit(1)

    files_dir = sys.argv[1]
    app.run(host="0.0.0.0", port=4570)
