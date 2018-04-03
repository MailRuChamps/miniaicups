FROM ubuntu:16.04
MAINTAINER Alexey Bolshakov <ua3mqj@gmail.com>

ENV OTP_VERSION="20.3.2"

RUN set -xe \
  && runtimeDeps='libodbc1 \
      libsctp1 \
      libwxgtk3.0' \
  && buildDeps='unixodbc-dev \
      libsctp-dev \
      libwxgtk3.0-dev' \
  && apt-get update \
  && apt-get install -y curl \
  && apt-get install -y autoconf \
  && apt-get install -y build-essential

RUN set -xe \
  && OTP_DOWNLOAD_URL="https://github.com/erlang/otp/archive/OTP-${OTP_VERSION}.tar.gz" \
  && OTP_DOWNLOAD_SHA256="9809be52baa23d6fd18ee70b9a9b7c548e44f586db2f46ff5bfe66719cfab10a" \
  && curl -fSL -o otp-src.tar.gz "$OTP_DOWNLOAD_URL" \
  && echo "$OTP_DOWNLOAD_SHA256  otp-src.tar.gz" | sha256sum -c - \
  && export ERL_TOP="/usr/src/otp_src_${OTP_VERSION%%@*}" \
  && mkdir -vp $ERL_TOP \
  && tar -xzf otp-src.tar.gz -C $ERL_TOP --strip-components=1 \
  && rm otp-src.tar.gz

RUN set -xe \
  && apt-get -y install m4 \
  && apt-get -y install libncurses5-dev \
  && apt-get -y install libssh-dev \
  && apt-get -y install libxml2-utils \
  && apt-get -y install fop \
  && apt-get -y install xsltproc \
  && apt-get -y install alien \
  && apt-get -y install libaio1

RUN set -xe \
  && export ERL_TOP="/usr/src/otp_src_${OTP_VERSION%%@*}" \
  && ( cd $ERL_TOP \
    && ./otp_build autoconf \
    && ./configure \
    && make \
    && make install ) \
  && find /usr/local -name examples | xargs rm -rf \
  && apt-get purge -y --auto-remove $buildDeps \
  && rm -rf $ERL_TOP /var/lib/apt/lists/*

ENV ELIXIR_VERSION="v1.6.4" \
  LANG=C.UTF-8

RUN set -xe \
  && ELIXIR_DOWNLOAD_URL="https://github.com/elixir-lang/elixir/archive/${ELIXIR_VERSION}.tar.gz" \
  && ELIXIR_DOWNLOAD_SHA256="c12a4931a5383a8a9e9eb006566af698e617b57a1f645a6cb132a321b671292d" \
  && curl -fSL -o elixir-src.tar.gz $ELIXIR_DOWNLOAD_URL \
  && echo "$ELIXIR_DOWNLOAD_SHA256  elixir-src.tar.gz" | sha256sum -c - \
  && mkdir -p /usr/local/src/elixir \
  && tar -xzC /usr/local/src/elixir --strip-components=1 -f elixir-src.tar.gz \
  && rm elixir-src.tar.gz \
  && cd /usr/local/src/elixir \
  && make install clean

COPY mix.exs ./
COPY main.ex ./lib

RUN mix local.hex --force
RUN mix deps.get

ENV COMPILED_FILE_PATH=/opt/client/main
ENV SOLUTION_CODE_ENTRYPOINT=elixir_strategy
ENV COMPILATION_COMMAND='mix escript.build'
ENV RUN_COMMAND='elixir_strategy'
