FROM ubuntu:16.04
MAINTAINER Konstantin Aristov <kostya.74.74@gmail.com>

RUN apt-get update -y && \
    apt-get install -y apt-utils apt-transport-https software-properties-common && \
    apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 2EE0EA64E40A89B84B2DF73499E82A75642AC823 && \
    add-apt-repository "deb https://dl.bintray.com/sbt/debian /" && \
    apt-get update -y && \
    apt-get install -y scala sbt
    chmod a+rx /root
	
RUN mkdir -p /opt/client/src/main/scala

COPY src /opt/client/src
COPY build.sbt /opt/client

ENV SOLUTION_CODE_ENTRYPOINT=Main.scala
ENV COMPILED_FILE_PATH=/opt/client/
ENV SOLUTION_CODE_PATH=/opt/client/src/main/scala/
ENV COMPILATION_COMMAND='sbt compile'
ENV RUN_COMMAND='sbt run'