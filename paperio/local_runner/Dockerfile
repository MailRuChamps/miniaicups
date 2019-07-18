FROM ubuntu:18.04

WORKDIR /opt/mechanic

RUN apt-get update && \
    apt-get install -y python3.6 python3-pip && \
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove

COPY requirements.txt ./requirements.txt
RUN pip3 install -r requirements.txt

COPY . ./

EXPOSE 8000
CMD ["python3", "-u", "./serverrunner.py"]