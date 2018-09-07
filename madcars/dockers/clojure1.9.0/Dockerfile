FROM ubuntu:16.04
MAINTAINER Ivan Smolyakov <smol.ivan97@gmail.com>

RUN apt-get update -y \
    && apt-get install -y openjdk-8-jdk \
    && apt-get install -y wget \
    && wget "https://raw.githubusercontent.com/technomancy/leiningen/stable/bin/lein" -O /usr/bin/lein \
    && chmod +x /usr/bin/lein 


ENV COMPILED_FILE_PATH=/opt/client/target/client.jar
ENV SOLUTION_CODE_ENTRYPOINT=client/core.clj
ENV SOLUTION_CODE_PATH=/opt/client/src
ENV COMPILATION_COMMAND='lein uberjar'
ENV RUN_COMMAND='java -jar $MOUNT_POINT'

COPY project.clj /opt/client/project.clj
RUN mkdir -p /opt/client/src \
    && cd /opt/client \
    && lein deps

