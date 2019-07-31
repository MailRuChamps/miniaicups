FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN add-apt-repository ppa:jonathonf/python-3.6 && \
  apt-get update && apt-get install -y python3.6 python3-pip && \
  python3.6 -m pip install -U numpy scipy cython scikit-learn pandas

ENV SOLUTION_CODE_ENTRYPOINT=main.py
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='python3.6 -u $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'
