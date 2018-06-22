Microservices and OpenTracing
=============================

# About

A setup for demonstrational purposes on how to integrate OpenTracing
into a microservice-like setup.

Setup components include Ruby, Python, Go, C++ and NodeJS
implementations. We use Zipkin as tracing-backend.

![Code neighbouring](https://raw.github.com/MarcelHB/opentracing-demo/master/screenshots/setup.png)

# Setup

Obtain docker and docker-compose. Then do:

```
$ docker-compose up
```

The following service ports are exposed under your docker host:

* 4567: a trace-initiator
* 9000: Portainer, a docker setup monitor
* 9411: Zipkin, the tracing-backend

Access the service on 4567 to make things visible in Zipkin.

# Authors and License

Dennis Struhs, Marcel Heing-Becker

Any resource here is subject to the The Unlicense.
