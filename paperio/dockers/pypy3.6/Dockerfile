FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN add-apt-repository ppa:pypy/ppa -y && \
    apt-get update && apt-get install pypy -y

ENV SOLUTION_CODE_ENTRYPOINT=main.py
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='pypy -u $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'