FROM ubuntu:16.04
MAINTAINER Sergey Elkin <si.elkin@mail.ru>

RUN apt-get update

RUN apt-get install -y curl libgdbm-dev libncurses5-dev automake libtool bison libffi-dev

# Install ruby 2.5.0 via rvm
RUN gpg --keyserver hkp://keys.gnupg.net --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 7D2BAF1CF37B13E2069D6956105BD0E739499BDB
RUN \curl -L https://get.rvm.io | bash -s stable
RUN /bin/bash -l -c "rvm requirements"
RUN /bin/bash -l -c "rvm install 2.5.0"
RUN /bin/bash -l -c "gem install bundler --no-ri --no-rdoc"

# Please use the following structure for your ruby solution:
#
# client/
#   bin/
#     solution            #Run your app there
#   lib/
#     app_name.rb          #replace app_name with your own Application name
#     app_name/            #Classes and so on
#   Gemfile

ENV SOLUTION_CODE_ENTRYPOINT=solution
ENV SOLUTION_CODE_PATH=/opt/client

RUN touch /usr/run.sh

# run `bundle install` before application launch
RUN echo "cd $SOLUTION_CODE_PATH" > /usr/run.sh
RUN echo "/bin/bash -l -c \"bundle install\"" >> /usr/run.sh

# Please pay attention on entry point location, it is /opt/client/bin/solution
RUN echo "/bin/bash -l -c \"irb \"./bin/$SOLUTION_CODE_ENTRYPOINT\"\"" >> /usr/run.sh

ENV RUN_COMMAND="sh /usr/run.sh"
