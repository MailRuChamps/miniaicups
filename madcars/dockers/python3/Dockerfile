FROM ubuntu:16.04
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN apt-get update && apt-get install -y python3 python3-pip && pip3 install -U numpy scipy cython scikit-learn keras pandas tensorflow==1.5.0 pymunk==5.3.2

ENV SOLUTION_CODE_ENTRYPOINT=main.py
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='python3 -u $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'
