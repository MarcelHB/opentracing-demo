package main

import (
	"net/http"
	"os"

	opentracing "github.com/opentracing/opentracing-go"
	zipkin "github.com/openzipkin/zipkin-go-opentracing"
	"github.com/valyala/fasthttp"
)

var (
	tracer opentracing.Tracer = nil
)

func httpHandler(ctx *fasthttp.RequestCtx) {
	headers := http.Header{}
	ctx.Request.Header.VisitAll(func(key, value []byte) {
		headers.Set(string(key[:]), string(value[:]))
	})

	parent_context, err := tracer.Extract(
		opentracing.HTTPHeaders,
		opentracing.HTTPHeadersCarrier(headers),
	)

	if err != nil {
		ctx.Response.SetStatusCode(500)
		return
	}

	span := opentracing.StartSpan("post_processor", opentracing.ChildOf(parent_context))
	span.SetTag("span.kind", "server")
	span.SetTag("component", "post processor")
	span.SetTag("http.url", "http://post_processor:4567/")
	defer span.Finish()

	ctx.WriteString("{\"status\":\"ok\"}")
}

func main() {
	if os.Getenv("ZIPKIN_HOST") == "" {
		os.Stderr.WriteString("Please set ZIPKIN_HOST!\n")
		os.Exit(-1)
	}
	zipkin_host := "http://" + os.Getenv("ZIPKIN_HOST") + "/api/v1/spans"

	collector, err := zipkin.NewHTTPCollector(zipkin_host)
	if err != nil {
		os.Exit(-1)
	}
	defer collector.Close()

	recorder := zipkin.NewRecorder(collector, false, "", "demo_arch")

	_tracer, err := zipkin.NewTracer(recorder)
	if err != nil {
		os.Exit(-1)
	}
	tracer = _tracer

	opentracing.InitGlobalTracer(tracer)

	fasthttp.ListenAndServe("0.0.0.0:4567", httpHandler)
}
