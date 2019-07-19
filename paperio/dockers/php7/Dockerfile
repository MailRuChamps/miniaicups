FROM stor.highloadcup.ru/aicups/paperio_base
MAINTAINER Boris Kolganov <b.kolganov@corp.mail.ru>

RUN DEBIAN_FRONTEND=noninteractive LC_ALL=en_US.UTF-8 add-apt-repository -y ppa:ondrej/php && \
    apt-get update && \
    apt-get -y install php7.1 php7.1-cli php7.1-common php7.1-json php7.1-mbstring

ENV SOLUTION_CODE_ENTRYPOINT=main.php
ENV SOLUTION_CODE_PATH=/opt/client/solution
ENV RUN_COMMAND='/usr/bin/php $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT'
