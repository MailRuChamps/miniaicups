FROM ubuntu:16.04
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN add-apt-repository -y ppa:longsleep/golang-backports && \
    apt-get update && \
    apt-get install -y golang-go

RUN go install -buildmode=shared std

ENV COMPILED_FILE_PATH=/opt/client/main
ENV SOLUTION_CODE_ENTRYPOINT=main.go
ENV COMPILATION_COMMAND='go build -linkshared $SOLUTION_CODE_PATH/main.go  2>&1 > /dev/null'
ENV RUN_COMMAND='/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT'
