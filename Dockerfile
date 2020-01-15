FROM    debian:stretch
LABEL   maintainer="JES <je@schober.industries>" \
        version="1.0"
ENV     DEBIAN_FRONTEND noninteractive
ENV     LD_LIBRARY_PATH /usr/local/lib:/tmp/usr/local/lib
RUN     apt-get clean && apt-get update
# Basic tools
RUN     apt-get install -y \
        gcc g++ \
        libc-dev \
        make \
        curl wget git
# Dependencies of wjwwood's serial library
RUN     apt-get install -y \
        cmake \
        python python-pip \
        catkin
# Mosquitto
RUN     apt-get install -y \ 
        libmosquittopp-dev
RUN     mkdir /repos
WORKDIR /repos
# Install wjwwood's serial library
RUN     git clone https://github.com/wjwwood/serial.git
WORKDIR /repos/serial
RUN     make -j $(nproc)
RUN     make install
# Install and build the serial gateway
RUN     mkdir /repos/Serial2MqttGateway
COPY    ./ /repos/Serial2MqttGateway/
WORKDIR /repos/Serial2MqttGateway/
RUN     chmod +x build.sh
RUN     ["/bin/bash", "-c", "./build.sh"]