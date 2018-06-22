FROM ubuntu:artful

RUN apt-get update -qq
RUN apt-get install -y git build-essential g++ cmake libcurl4-gnutls-dev

RUN mkdir /Source
WORKDIR /Source

RUN git clone https://github.com/opentracing/opentracing-cpp
# best repo doesn't support Log(), that's an open PR
RUN git clone https://github.com/jmcomets/zipkin-cpp-opentracing --branch logs

RUN mkdir /Source/opentracing-cpp/build
WORKDIR /Source/opentracing-cpp/build
RUN cmake ..
RUN make && make install

RUN mkdir /Source/zipkin-cpp-opentracing/build
WORKDIR /Source/zipkin-cpp-opentracing/build
RUN cmake ..
RUN make && make install

RUN mkdir /Source/spooler
WORKDIR /Source/spooler

COPY *.cpp ./
COPY *.h ./
COPY CMakeLists.txt ./

RUN mkdir /Source/spooler/build
WORKDIR /Source/spooler/build
RUN cmake ..
RUN make

RUN mkdir -p /app/files
RUN mv ./spooler /app
WORKDIR /app

ENV LD_LIBRARY_PATH=/usr/local/lib
