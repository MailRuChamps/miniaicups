FROM ubuntu:16.04
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN \
    apt-get update -y && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update -y && \
    apt-get install -y g++-7 make

COPY Makefile ./
COPY ./nlohmann ./nlohmann

ENV SOLUTION_CODE_ENTRYPOINT=main.cpp
ENV COMPILED_FILE_PATH=/opt/client/a.out
ENV SOLUTION_CODE_PATH=/opt/client/solution/
ENV COMPILATION_COMMAND='make 2>&1 > /dev/null'
ENV RUN_COMMAND='/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT'
