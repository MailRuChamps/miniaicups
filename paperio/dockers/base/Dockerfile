FROM ubuntu:16.04
MAINTAINER Kislenko Maksim <m.kislenko@corp.mail.ru>

ENV MOUNT_POINT=/opt/client/code
ENV SOLUTION_CODE_PATH=/opt/client/solution

RUN apt-get update && \
    apt-get install -y unzip curl software-properties-common language-pack-en-base build-essential qt5-default python && \
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove

WORKDIR /opt/client
COPY ./sources ./
RUN qmake ./client.pro -r CONFIG+=x86_64 && make && rm -f Makefile client.pro constants.h main.cpp main.o moc_tcp_client.cpp moc_tcp_client.o tcp_client.h
RUN mkdir /opt/client/solution && chmod 777 /opt/client/solution

CMD ["bash", "run.sh"]
