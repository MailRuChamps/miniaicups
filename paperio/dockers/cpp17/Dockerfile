FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN \
    apt-get update -y && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update -y && \
    apt-get install -y g++-7 make cmake

RUN \
    curl https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-latest.zip -o /libtorch.zip && \
    ln -s /usr /libtorch && unzip /libtorch.zip "libtorch/lib/*" "libtorch/share/*" "libtorch/include/*" -d / && \
    rm -rf /libtorch.zip /libtorch

COPY Makefile ./
COPY ./nlohmann ./nlohmann

ENV SOLUTION_CODE_ENTRYPOINT=main.cpp
ENV COMPILED_FILE_PATH=/opt/client/a.out
ENV SOLUTION_CODE_PATH=/opt/client/solution/
ENV COMPILATION_COMMAND='if [ -f $SOLUTION_CODE_PATH/__build__.sh ]; then cd $SOLUTION_CODE_PATH; . $SOLUTION_CODE_PATH/__build__.sh; else make; fi 2>&1 > /dev/null'
ENV RUN_COMMAND='/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT'
