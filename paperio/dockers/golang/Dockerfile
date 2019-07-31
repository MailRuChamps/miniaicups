FROM stest.tech-mail.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN apt-get update -y && apt-get install --no-install-recommends -y -q curl build-essential ca-certificates git mercurial bzr
RUN mkdir /goroot && curl https://dl.google.com/go/go1.12.7.linux-amd64.tar.gz | tar xvzf - -C /goroot --strip-components=1
RUN mkdir /gopath

ENV GOROOT /goroot
ENV GOPATH /gopath
ENV PATH $GOROOT/bin:$GOPATH/bin:$PATH

RUN go install -buildmode=shared std

ENV COMPILED_FILE_PATH=/opt/client/main
ENV SOLUTION_CODE_ENTRYPOINT=main.go
ENV COMPILATION_COMMAND='go build -linkshared $SOLUTION_CODE_PATH/main.go  2>&1 > /dev/null'
ENV RUN_COMMAND='/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT'
