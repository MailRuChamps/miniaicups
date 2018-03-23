FROM ubuntu:16.04
MAINTAINER Evgeniy Zuykin <eugenzuy@gmail.com>
RUN apt-get update -y && \
    apt-get install -y software-properties-common && \
    add-apt-repository -y ppa:cwchien/gradle && \
    apt-get update -y && \
    apt-get install -y openjdk-8-jdk gradle-3.5.1  && \
    rm -rf /var/lib/apt/lists/* && \
    chmod a+rx /root

ENV SOLUTION_CODE_ENTRYPOINT=Main.Kt
ENV COMPILED_FILE_PATH=/opt/client/kotlinStrategy.jar
ENV SOLUTION_CODE_PATH=/opt/client/src/main/kotlin/
ENV COMPILATION_COMMAND='gradle build -q'
ENV RUN_COMMAND='java -jar $MOUNT_POINT'

COPY build.gradle ./
RUN mkdir -p src/main/kotlin && gradle build -q && \
    mv build/libs/kotlinStrategy.jar kotlinStrategy.jar && \
    rm -rf build
