FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Konstantin Aristov <kostya.74.74@gmail.com>

WORKDIR /opt/client

RUN apt-get update -y && \
    apt-get install -y apt-utils apt-transport-https software-properties-common && \
    apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 2EE0EA64E40A89B84B2DF73499E82A75642AC823 && \
    add-apt-repository "deb https://dl.bintray.com/sbt/debian /" && \
    apt-get update -y && \
    apt-get install -y scala sbt && \
    rm -rf /var/lib/apt/lists/* && \
    mkdir -p /opt/client/src/main/scala

COPY build.sbt /opt/client
COPY Main.scala /opt/client/src/main/scala/.
COPY assembly.sbt /opt/client/project/assembly.sbt
RUN sbt assembly && rm -rf target && rm -rf /opt/client/src/main/scala/*

ENV SOLUTION_CODE_ENTRYPOINT=Main.scala
ENV COMPILED_FILE_PATH=/opt/client/target/scala-2.11/Strategy-assembly-0.1.0.jar
ENV SOLUTION_CODE_PATH=/opt/client/src/main/scala
ENV COMPILATION_COMMAND='sbt -error assembly'
ENV RUN_COMMAND='java -jar $MOUNT_POINT'
