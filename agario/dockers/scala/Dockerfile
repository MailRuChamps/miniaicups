FROM ubuntu:16.04
MAINTAINER Konstantin Aristov <kostya.74.74@gmail.com>

RUN wget http://scala-lang.org/files/archive/scala-2.12.5.deb && \
	dpkg -i scala-2.12.5.deb && \
    apt-get update && \
    apt-get install -y scala

ENV COMPILED_FILE_PATH=/opt/client/main
ENV SOLUTION_CODE_ENTRYPOINT=Main.scala
ENV COMPILATION_COMMAND='scalac $SOLUTION_CODE_PATH/*'
ENV RUN_COMMAND='scala Main'