FROM ubuntu:16.04
MAINTAINER Cthulhu <cthulhu.den@gmail.com>

RUN apt-get update -qq && apt-get install -y ghc

ENV COMPILED_FILE_PATH=/opt/client/a.out
ENV SOLUTION_CODE_ENTRYPOINT=Main.hs
ENV SOLUTION_CODE_PATH=/opt/client/solution

ENV COMPILATION_COMMAND='ghc -O2 -i$SOLUTION_CODE_PATH -o $COMPILED_FILE_PATH $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT > /dev/null'
ENV RUN_COMMAND='/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT'
