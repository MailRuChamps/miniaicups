FROM ubuntu:16.04
MAINTAINER Sergey Samokhvalov <onlyforthesky@gmail.com>

ENV DEBIAN_FRONTEND=noninteractive
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

ENV SOLUTION_CODE_ENTRYPOINT=solution.cs
ENV SOLUTION_CODE_PATH=/opt/client/solution/

RUN apt-get update && \
    apt-get install -y apt-transport-https apt-utils wget

RUN wget -q https://packages.microsoft.com/config/ubuntu/16.04/packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb && \
    apt-get update && \
    apt-get install -y zip unzip dotnet-sdk-2.1 && \
    mkdir -p /opt/client/solution && \
    rm -rf packages-microsoft-prod.deb /var/lib/apt/lists/* /tmp/*

ENV COMPILED_FILE_PATH=/opt/client/out.dll
ENV COMPILATION_COMMAND='cd /opt/client/solution/ && if [ -f __build__.sh ]; then . __build__.sh; else . ../default/__build__.sh; fi'
ENV RUN_COMMAND='cd /opt/client/solution/ && if [ -f __run__.sh ]; then . __run__.sh; else . ../default/__run__.sh; fi'

COPY Newtonsoft.Json.dll /opt/client/libs/Newtonsoft.Json.dll
COPY solution.csproj /opt/client/default/solution.csproj
COPY __build__.sh /opt/client/default/__build__.sh
COPY __run__.sh /opt/client/default/__run__.sh
