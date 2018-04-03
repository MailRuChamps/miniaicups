FROM ubuntu:16.04
MAINTAINER Alexey Bolshakov <ua3mqj@gmail.com>

RUN set -xe \
  && apt-get update \
  && apt-get install -y wget \
  && wget https://packages.erlang-solutions.com/erlang-solutions_1.0_all.deb \
  && dpkg -i erlang-solutions_1.0_all.deb \
  && wget https://packages.erlang-solutions.com/ubuntu/erlang_solutions.asc \
  && apt-key add erlang_solutions.asc \
  && apt-get update \
  && apt-get install -y elixir

ENV COMPILED_FILE_PATH=/opt/client/strategy
ENV SOLUTION_CODE_ENTRYPOINT=strategy
ENV COMPILATION_COMMAND='mix escript.build'
ENV RUN_COMMAND='escript $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'
