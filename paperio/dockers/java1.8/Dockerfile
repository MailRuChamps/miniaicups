FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>
RUN apt-get update -y && \
    apt-get install -y maven software-properties-common curl && \
    echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | debconf-set-selections && \
    add-apt-repository -y ppa:webupd8team/java && \
    apt-get update -y && \
    apt-get install -y oracle-java8-installer python && \
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /var/cache/oracle-jdk8-installer && \
    chmod a+rx /root

ENV SOLUTION_CODE_ENTRYPOINT=Main.java
ENV COMPILED_FILE_PATH=/opt/client/javaStrategy.jar
ENV SOLUTION_CODE_PATH=/opt/client/src/main/java/
ENV COMPILATION_COMMAND='mvn package -q'
ENV RUN_COMMAND='java -jar $MOUNT_POINT'

COPY pom.xml ./
RUN mkdir -p src/main/java && mvn dependency:go-offline && \
    mvn package && \
    rm -rf javaStrategy.jar target/classes/
