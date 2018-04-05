FROM ubuntu:16.04
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

ENV MONO_GC_PARAMS max-heap-size=256M

ENV COMPILED_FILE_PATH=/opt/client/csharpStrategy

ENV NUGET_PACKAGES_DIR=./pkgs/
ENV NUGET_FILE_PATH=./nuget.exe
ENV NUGET_PACKAGES_CONFIG=./packages.config

ENV COMPILATION_COMMAND='csc $(find $NUGET_PACKAGES_DIR -wholename "*/net45/*.dll"  -exec echo "/reference:{}" \; | tr "\n" " " ) $(find $SOLUTION_CODE_PATH -name "*.cs") -out:$COMPILED_FILE_PATH'

ENV RUN_COMMAND='export MONO_PATH=$LD_LIBRARY_PATH:$(find $NUGET_PACKAGES_DIR -wholename "*/net45/*.dll"  -printf "%h/\n" | tr "\n" ":" ) && mono $MOUNT_POINT'

RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
RUN apt-get update && apt-get install -y wget

RUN echo "deb http://download.mono-project.com/repo/ubuntu stable-trusty main" > /etc/apt/sources.list.d/mono-official-stable.list && \
    apt-get update && \
    apt-get install -y mono-complete zip && \
    rm -rf /var/lib/apt/lists/* /tmp/* && \
    export MONO_GC_PARAMS=max-heap-size=256M

RUN bash -c "$(/bin/echo -e "cat > $NUGET_PACKAGES_CONFIG <<EOM\
    \n<?xml version=\"1.0\" encoding=\"utf-8\"?> \
    \n<packages> \
    \n    <package id=\"Accord\" version=\"3.8.0\" targetFramework=\"net45\" /> \
    \n    <package id=\"Accord.Genetic\" version=\"3.8.0\" targetFramework=\"net45\" /> \
    \n    <package id=\"Accord.MachineLearning\" version=\"3.8.0\" targetFramework=\"net45\" /> \
    \n    <package id=\"Accord.Math\" version=\"3.8.0\" targetFramework=\"net45\" /> \
    \n    <package id=\"Accord.Neuro\" version=\"3.8.0\" targetFramework=\"net45\" /> \
    \n    <package id=\"Accord.Statistics\" version=\"3.8.0\" targetFramework=\"net45\" /> \
    \n    <package id=\"Newtonsoft.Json\" version=\"11.0.2\" targetFramework=\"net45\" /> \
    \n</packages> \
    \nEOM\n")"

RUN wget https://dist.nuget.org/win-x86-commandline/latest/nuget.exe -O $NUGET_FILE_PATH && \
    cat packages.config &&\
    mono $NUGET_FILE_PATH install $NUGET_PACKAGES_CONFIG  -OutputDirectory $NUGET_PACKAGES_DIR
