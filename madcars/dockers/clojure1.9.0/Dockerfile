FROM ubuntu:16.04
MAINTAINER Ivan Smolyakov <smol.ivan97@gmail.com>

RUN apt-get update -y \
    && apt-get install -y openjdk-8-jdk \
    && apt-get install -y wget \
    && wget "https://raw.githubusercontent.com/technomancy/leiningen/stable/bin/lein" -O /usr/bin/lein \
    && chmod +x /usr/bin/lein 

ENV MOUNT_POINT=/opt/client
ENV SOLUTION_CODE_ENTRYPOINT=client/core.clj
ENV SOLUTION_CODE_PATH=$MOUNT_POINT/src
ENV COMPILATION_COMMAND='lein uberjar'
ENV RUN_COMMAND='java -jar $MOUNT_POINT/target/client.jar'

COPY project.clj $MOUNT_POINT/project.clj
RUN mkdir -p SOLUTION_CODE_PATH \
    && cd $MOUNT_POINT \
    && lein deps

