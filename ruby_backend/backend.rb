require "json"
require "sinatra"
require "zipkin/tracer"

set :port, 4567
set :bind, "0.0.0.0"

zipkin_url_base = "http://#{ENV["ZIPKIN_HOST"]}"

OpenTracing.global_tracer = Zipkin::Tracer.build(url: zipkin_url_base, service_name: "demo_arch")

before do
  parent_context = OpenTracing.extract(OpenTracing::FORMAT_RACK, request.env)
  @app_span = OpenTracing.start_span("auth_service", child_of: parent_context)
  @app_span.set_tag("span.kind", "server")
  @app_span.set_tag("component", "auth service")
  @app_span.set_tag("http.url", "http://auth_backend:4567/")
end

after do
  @app_span.finish
end

get "/" do
  if rand > 0.75
    @app_span.set_tag("http.status_code", 401)
    return 401
  end

  user_id = rand(100)
  @app_span.set_tag("user.id", user_id)
  @app_span.set_tag("http.status_code", 200)

  JSON.dump({ id: user_id })
end
