FROM ubuntu:16.04
MAINTAINER Kislenko Maksim <m.kislenko@corp.mail.ru>

RUN apt-get update && \
    apt-get install -y build-essential qt5-default && \
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove

WORKDIR /opt/mechanic
COPY . ./

RUN qmake ./server_runner.pro -r CONFIG+=x86_64\ release
RUN make

EXPOSE 8000
CMD ["./server_runner"]
