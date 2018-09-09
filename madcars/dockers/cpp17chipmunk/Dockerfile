FROM ubuntu:16.04
MAINTAINER Tony Kozlovsky <tonykozlovsky@gmail.com>

RUN \
    apt-get update -y && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update -y && \
    apt-get install -y g++-7 build-essential make

ENV COMPILED_FILE_PATH=/opt/client/a.out
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV SOLUTION_LIBRARY_PATH=$SOLUTION_CODE_PATH/chipmunk_src
ENV COMPILATION_COMMAND='make -C $SOLUTION_LIBRARY_PATH -f /opt/client/LibMakefile 2>&1 > /dev/null && make -C $SOLUTION_CODE_PATH/solution -f /opt/client/SolutionMakefile 2>&1 > /dev/null'
ENV RUN_COMMAND='/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT'

COPY ./nlohmann ./nlohmann
COPY ./LibMakefile ./
COPY ./SolutionMakefile ./
