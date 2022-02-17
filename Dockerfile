FROM    debian:stretch
LABEL   maintainer="JES <je@schober.industries>" \
        version="1.0"
ENV     DEBIAN_FRONTEND noninteractive
ENV     LD_LIBRARY_PATH /usr/local/lib:/tmp/usr/local/lib
RUN     apt-get clean && apt-get update
# Basic tools
RUN     apt-get install -y \
        build-essential \
        gcc g++ \
        libc-dev \
        make cmake \
        curl wget git \
        libssl-dev
# Dependencies of wjwwood's serial library
RUN     apt-get install -y \
        cmake \
        python python-pip \
        catkin
# Make repos directory
RUN     mkdir /repos
# Install Paho MQTT C
WORKDIR /repos
RUN     git clone https://github.com/eclipse/paho.mqtt.c.git
WORKDIR /repos/paho.mqtt.c
RUN     git checkout v1.3.9
RUN     cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF -DPAHO_BUILD_STATIC=ON -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON
RUN     cmake --build build/ --target install
RUN     ldconfig
# Install Paho MQTT C++
WORKDIR /repos
RUN     git clone https://github.com/eclipse/paho.mqtt.cpp.git
WORKDIR /repos/paho.mqtt.cpp
RUN     git checkout v1.2.0
RUN     cmake -Bbuild -H. -DPAHO_BUILD_STATIC=ON -DPAHO_WITH_SSL=ON
RUN     cmake --build build/ --target install
RUN     ldconfig
# Install wjwwood's serial library
WORKDIR /repos
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