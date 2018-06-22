FROM golang:1.10.1

RUN go get -v github.com/valyala/fasthttp
RUN go get -v github.com/openzipkin/zipkin-go-opentracing

RUN mkdir /service
WORKDIR /service
COPY *.go ./

RUN go build main.go

