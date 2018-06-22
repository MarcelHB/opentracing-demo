require "net/http"
require "uri"
require "sinatra"
require "zipkin/tracer"

HTML_TEMPLATE = <<-HTML
<html style="height: 100%">
  <head>
    <title>OpenTracing Demo-Frontend</title>
  </head>
  <body style="height: 100%">
    <div style="display: table; width: 100%; height: 100%">
      <div style="display: table-row">
        <div style="display: table-cell; vertical-align: middle; text-align: center">
          <em>Frontend says:</em>
          <h1>{{MESSAGE}}</h1>
        </div>
      </div>
    </div>
  </body>
</html>
HTML

def ping_backend(host)
  uri = URI.parse("http://#{host}")

  Net::HTTP.start(uri.host, uri.port) do |http|
    request = Net::HTTP::Get.new(uri)
    OpenTracing.inject(@app_span.context, OpenTracing::FORMAT_RACK, request)

    response = http.request(request)
    response.is_a?(Net::HTTPSuccess)
  end
end

def ping_backend1
  ping_backend(ENV["BACKEND1_HOST"])
end

def ping_backend2
  ping_backend(ENV["BACKEND2_HOST"])
end

def ping_backend3
  ping_backend(ENV["BACKEND3_HOST"])
end

def ping_backend4
  ping_backend(ENV["BACKEND4_HOST"])
end

def render_html(message)
  HTML_TEMPLATE.gsub(/{{MESSAGE}}/, message)
end

zipkin_url_base = "http://#{ENV["ZIPKIN_HOST"]}"

OpenTracing.global_tracer = Zipkin::Tracer.build(url: zipkin_url_base, service_name: "demo_arch")

set :port, 4567
set :bind, "0.0.0.0"

before do
  @app_span = OpenTracing.start_span("app")
  @app_span.set_tag("span.kind", "server")
  @app_span.set_tag("component", "frontend")
  @app_span.set_tag("http.user_agent", request.env["HTTP_USER_AGENT"])
  @app_span.set_tag("http.url", "http://frontend:4567/")
end

after do
  @app_span.finish
end

get "/" do
  @app_span.log_kv(event: "auth")
  unless ping_backend1
    @app_span.log_kv({ event: "error", :"error.kind" => "auth failure" })
    @app_span.set_tag("http.status_code", 401)
    return [401, [render_html("Bad user auth!")]]
  end

  @app_span.log_kv(event: "data")
  ping_backend2

  @app_span.log_kv(event: "calculation")
  ping_backend3

  @app_span.log_kv(event: "post-processing")
  ping_backend4

  @app_span.set_tag("http.status_code", 200)

  render_html("We should have launched a trace.")
end
