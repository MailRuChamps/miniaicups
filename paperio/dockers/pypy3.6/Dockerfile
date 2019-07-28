FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN curl -sSL https://bitbucket.org/pypy/pypy/downloads/pypy3.6-v7.1.1-linux64.tar.bz2 | tar xjv -C /usr/local --strip-components=1 && \
    pypy3 -m ensurepip && pypy3 -m pip install --upgrade pip setuptools wheel && \
    pypy3 -m pip install numpy && pypy3 -c 'import numpy'

ENV SOLUTION_CODE_ENTRYPOINT=main.py
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='pypy3 -u $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'