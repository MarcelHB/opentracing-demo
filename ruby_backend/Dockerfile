FROM ruby:2.5.1

RUN apt-get update -qq && apt-get install -y build-essential

RUN mkdir /service
WORKDIR /service

COPY Gemfile* ./

RUN bundle install
