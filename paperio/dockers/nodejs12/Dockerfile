FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN curl -sL https://deb.nodesource.com/setup_12.x | bash - && \
    apt-get install -y nodejs

ENV SOLUTION_CODE_ENTRYPOINT=main.js
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='/usr/bin/node $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'
