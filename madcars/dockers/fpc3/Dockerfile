FROM ubuntu:16.04
MAINTAINER Igor Kuznetsov <garry_smith@mail.ru>

RUN apt-get update && apt-get install -y fpc

ENV COMPILED_FILE_PATH=/opt/client/main
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV SOLUTION_CODE_ENTRYPOINT=main.pp
ENV COMPILATION_COMMAND='fpc -Cs67107839 -Mobjfpc -XS $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT -o$COMPILED_FILE_PATH 2>&1 > /dev/null'
ENV RUN_COMMAND='$COMPILED_FILE_PATH'