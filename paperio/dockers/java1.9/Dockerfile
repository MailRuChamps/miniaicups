FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>
RUN apt-get update -y && \
    apt-get install -y maven && \
    apt-get -o Dpkg::Options::="--force-overwrite" install -y openjdk-9-jdk

ENV SOLUTION_CODE_ENTRYPOINT=Main.java
ENV COMPILED_FILE_PATH=/opt/client/javaStrategy.jar
ENV SOLUTION_CODE_PATH=/opt/client/src/main/java/
ENV COMPILATION_COMMAND='mvn package -q'
ENV RUN_COMMAND='java -jar $MOUNT_POINT'

COPY pom.xml ./
RUN mkdir -p src/main/java && mvn dependency:go-offline && \
    mvn package && \
    rm -rf javaStrategy.jar target/classes/
