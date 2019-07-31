FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Cthulhu <cthulhu.den@gmail.com>

ENV PATH=/opt/ghc/bin:$PATH

RUN apt-get update -qq && apt-get install -y software-properties-common && \
    add-apt-repository -y ppa:hvr/ghc && \
    apt-get update -qq && apt-get install -y ghc-8.4.1 cabal-install-2.2 && \
    cabal update && cabal install -j aeson classy-prelude conduit && \
    rm -rf /root/.cabal/packages # takes too much space, and we don't need it anymore

ENV COMPILED_FILE_PATH=/opt/client/a.out
ENV SOLUTION_CODE_ENTRYPOINT=Main.hs
ENV SOLUTION_CODE_PATH=/opt/client/solution

ENV COMPILATION_COMMAND='ghc -O2 -Wall -i$SOLUTION_CODE_PATH -o $COMPILED_FILE_PATH $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT 2>&1 > /dev/null'
ENV RUN_COMMAND='/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT'
