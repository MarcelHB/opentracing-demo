const express = require("express");
const app = express();

const ZipkinTracing = require("zipkin-javascript-opentracing");
const { BatchRecorder } = require("zipkin");
const { HttpLogger } = require("zipkin-transport-http");
const tags = require('opentracing').Tags;

const tracer = new ZipkinTracing({
  serviceName: "demo_arch",
  endpoint: "http://" + process.env.ZIPKIN_HOST,
  kind: "server"
});

app.use(function(request, response, next) {
  const parentContext = tracer.extract(
    ZipkinTracing.FORMAT_HTTP_HEADERS,
    request.headers
  );

  const span = tracer.startSpan("data_service", { childOf: parentContext });
  /* Die Implementierung von JS unterstützt nur genau einen Tag-Typ! :/ */
  /*
  span.setTag(tags.SPAN_KIND, "server");
  span.setTag(tags.COMPONENT, "data");
  span.setTag(tags.HTTP_URL, "http://data_backend:4567");
  span.setTag(tags.DB_TYPE, "sql");
  */
  /* Hack, um trotzdem etwas zu haben: */

  const data = {};
  data[tags.SPAN_KIND] = "server";
  data[tags.COMPONENT] = "data";
  data[tags.HTTP_URL] = "http://data_backend:4567";
  data[tags.DB_TYPE] = "sql";
  span.log(data);

  request.span = span;

  response.on("finish", function() {
    span.finish();
  });

  next();
});

app.get('/', function(request, response) {
  const someId = parseInt(Math.random() * 100);

  const data = {};
  data[tags.DB_STATEMENT] = `SELECT * FROM items WHERE items.parent_id = ${someId}`;
  data[tags.HTTP_STATUS_CODE] = 200;
  request.span.log(data);

  response.send(JSON.stringify({ "result": "success" }));
});

app.listen(4567);
