FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN apt-get update && \
    apt-get install -y clang

RUN apt-get install -y libcurl3 libpython2.7 libpython2.7-dev libcurl4-openssl-dev

RUN apt-get install -y wget && \
    wget https://swift.org/builds/swift-5.0.2-release/ubuntu1604/swift-5.0.2-RELEASE/swift-5.0.2-RELEASE-ubuntu16.04.tar.gz && \
    tar xzf swift-5.0.2-RELEASE-ubuntu16.04.tar.gz && \
    rm swift-5.0.2-RELEASE-ubuntu16.04.tar.gz && \
    mv swift-5.0.2-RELEASE-ubuntu16.04 /usr/share/swift

ENV SOLUTION_CODE_ENTRYPOINT=main.swift
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='/usr/share/swift/usr/bin/swift $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'