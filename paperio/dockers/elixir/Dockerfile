FROM stor.highloadcup.ru/aicups/paperio_base
LABEL authors="Alexey Bolshakov <ua3mqj@gmail.com>, Sergey Samokhvalov <onlyforthesky@gmail.com>"

COPY mix.exs /opt/client/default/mix.exs
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ERL_AFLAGS="+A 1 +S 2:2 +SDcpu 2:2"
ENV SOLUTION_CODE_PATH=/opt/client/solution/lib

RUN (echo 'deb http://packages.erlang-solutions.com/ubuntu xenial contrib' >> /etc/apt/sources.list) \
  && (apt-key adv --fetch-keys  http://packages.erlang-solutions.com/ubuntu/erlang_solutions.asc) \
  && apt-get update \
  && apt-get install -y wget elixir \
  && cd /opt/client/default \
  && mix local.hex --force \
  && mix deps.get \
  && mkdir -p $SOLUTION_CODE_PATH


ENV COMPILED_FILE_PATH=/opt/client/strategy
ENV SOLUTION_CODE_ENTRYPOINT=strategy.ex

ENV MIX_ENV=prod
ENV COMPILATION_COMMAND='cd /opt/client/solution/ && if [ ! -f "mix.exs" ]; then cp -r ../default/* .; fi  && mix escript.build 2>&1'
ENV RUN_COMMAND='/usr/bin/escript "$MOUNT_POINT"'
