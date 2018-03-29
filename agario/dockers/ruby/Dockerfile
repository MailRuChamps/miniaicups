FROM ubuntu:16.04
MAINTAINER Yasha Krasnov <y.krasnov@rubyroidlabs.com>

RUN apt-get update && apt-get install -y ruby ruby-dev ruby-bundler && rm -rf /var/lib/apt/lists/*

ENV SOLUTION_CODE_ENTRYPOINT=app.rb
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='ruby $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'
