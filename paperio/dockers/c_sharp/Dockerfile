FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

ENV MONO_GC_PARAMS max-heap-size=256M

ENV SOLUTION_CODE_ENTRYPOINT=main.cs
ENV COMPILED_FILE_PATH=/opt/client/csharpStrategy
ENV COMPILATION_COMMAND='csc /unsafe /reference:Newtonsoft.Json.dll `find $SOLUTION_CODE_PATH -name "*.cs"` -out:$COMPILED_FILE_PATH'
ENV RUN_COMMAND='mono $MOUNT_POINT'

RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
RUN echo "deb http://download.mono-project.com/repo/ubuntu stable-trusty main" > /etc/apt/sources.list.d/mono-official-stable.list && \
    apt-get update && \
    apt-get install -y mono-complete zip && \
    rm -rf /var/lib/apt/lists/* /tmp/* && \
    export MONO_GC_PARAMS=max-heap-size=256M

COPY Newtonsoft.Json.dll ./
