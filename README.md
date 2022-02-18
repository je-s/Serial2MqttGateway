# Serial2MqttGateway
The Serial2MqttGateway functions as a gateway between MQTT and serial devices, which can each be adressed and interfered with via unique topics based on the device's respective device ID.\
The application is based on the [SerialPortGateway](https://github.com/je-s/SerialPortGateway).

Features:
* All of SerialPortGateway's features:
    * Auto discovery of new serial devices
    * Processing of incoming messages from the devices
        * Easy to use message format for communicating with serial devices
    * Sending messages/commands to specific devices or all devices at once
    * Automatic handling of IO Errors
    * Logging of all actions
    * Designed as a base class for other applications to be build upon
        * Callbacks for easily handling different events (New messages, devices added/deleted )
        * Easy to use interface for interfering with the gateway
    * Parallelization of all time critical or potentially blocking tasks
    * Quick configuration of the gateway via easy config files
    * Can be used either dockerized or standalone
* Communication via MQTT with the serial devices
    * Messages coming from the devices are send to an MQTT broker.
        * Incoming messages are organized hierarchically in topics, based on their device ID and message type.
    * Commands can be delivered over MQTT to each device connected.
* Gateway-Commands, which can control the Serial2MqttGateway over MQTT.

# Table of Contens
1. [Files and Folder structure](#files-and-folder-structure)
2. [Dependencies](#dependencies)
3. [Installation](#installation)
4. [Including and compiling Serial2MqttGateway in a project](#including-and-compiling-serial2mqttgateway-in-a-project)
5. [Code Documentation](#code-documentation)
6. [Requirements for communicating between the Serial2MqttGateway and serial devices](#requirements-for-communicating-between-the-serial2mqttgateway-and-serial-devices)
7. [MQTT topic hierarchy](#mqtt-topic-hierarchy)
8. [Usage](#usage)
    1. [Configuration files](#configuration-files)
        1. [Main Config file](#main-config-file)
        2. [Hardware ID Whitelist](#hardware-id-whitelist)
        3. [Serial Port Blacklist](#serial-port-blacklist)
    2. [Starting the application in a Docker container](#starting-the-application-in-a-docker-container)
    3. [Communicating with the gateway](#communicating-with-the-gateway)
    4. [Communicating with the devices](#communicating-with-the-devices)
9. [Notes](#notes)
10. [License](#license)

# Files and Folder structure
* `config` contains the configuration files
    * Sample Config file
    * Sample hardware ID whitelist
    * Sample serial port blacklist
    * `cert` contains a sample cert (from Let's Encrypt, in case you have a service which uses a cert issued by them)
* `dependencies` is the place where all dependencies get downloaded to (See [Installation](#Installation) for further details)
* `src` contains the source code
    * `Serial2MqttGateway` class
    * `serial2mqtt-gateway` main application runtime
* `.env` is an environment file for Docker
* `.gitmodules` contains references to the dependencies
* `build.sh` is a script for building the application
* `clearMqttTopics.sh` is a script for clearing all retained MQTT topics
* `docker-compose.yml` is a docker-compose for easy deployment
* `Dockerfile` is the Dockerfile which can be used for building an image with all needed dependencies
* `Makefile` is for compiling the application

# Dependencies
If you follow the [Installation](#installation) instructions, some dependencies will be cloned directly into the dependencies folder:
* [SerialPortGateway](https://github.com/je-s/SerialPortGateway)
    * [Exception](https://github.com/je-s/Exception)
    * [Logger](https://github.com/je-s/Logger)
    * [Config](https://github.com/je-s/Config)

In case you're going to use the Docker image, you're good and don't require anything other than Docker in order to deploy/compile/test the Serial2MqttGateway or any enhancements done by yourself.

In case you want to use the Serial2MqttGateway without Docker, you need the following few things installed:
* gcc, g++, libc-dev, make, curl, wget, git.
    * g++ must be version 5 or higher.
    * git must be version 2 or higher.
* [wjwwood's serial library](https://github.com/wjwwood/serial). This also requires:
    * cmake, python, python-pip, catkin
* [paho.mqtt.c](https://github.com/eclipse/paho.mqtt.c.git)
    * must be version 1.3.9 or higher
* [paho.mqtt.cpp](https://github.com/eclipse/paho.mqtt.cpp.git)
    * must be version 1.2.0 or higher

If there's anything not working, try to sync your configuration with that from the Dockerfile (Or drop me an issue if nothing helps).

# Installation
1. Clone the repo
2. Change the directory into the repo
3. Execute `git submodule update --init --recursive --remote` to download all dependencies needed for using/compiling/executing the gateway
4. Building:
    1. Build the Docker image: `make buildDockerImage` or `docker-compose build`
    2. Build the application without Docker: `./build.sh`

# Including and compiling Serial2MqttGateway in a project
C++14 is required for compilation.

In order to use Serial2MqttGateway, `<path>/Serial2MqttGateway/src/Serial2MqttGateway.hpp` needs to be included.\
In order to compile Serial2MqttGateway, following files need to be compiled and linked:
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/dependencies/Logger/src/Logger.cpp`
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/dependencies/Logger/src/LogType.cpp`
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/dependencies/Exception/src/Exception.cpp`
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/dependencies/Config/src/ConfigExceptions.cpp`
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/dependencies/Config/src/Config.cpp`
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/src/SerialDevice.cpp`
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/src/SerialMessage.cpp`
* `<path>/Serial2MqttGateway/dependencies/SerialPortGateway/src/SerialPortGateway.cpp`

(Take a look at the Makefile.)

# Code Documentation
Besides this document, you can find detailed documentation in the [SerialPortGateway](https://github.com/je-s/SerialPortGateway).\
The sources are also documented where necessary/useful, but everything that's not documented in the SerialPortGateway should be pretty self explanatory.

# Requirements for communicating between the Serial2MqttGateway and serial devices
* Messages from devices need to follow the [SerialMessageFormat](https://github.com/je-s/SerialMessageFormat).
* The command declared via the config key `COMMAND_GETID` needs to be understood by every device connected and result in a validly formatted message from the device to the gateway directly after. This message needs to correspond to the message type declared with the config key `MESSAGE_TYPE_ID` and needs to contain the device's ID.
* The message delimiter declared with the config key `MESSAGE_DELIMITER` needs to be in sync with the message delimiter the connected devices are using.
* The baud rate declared with the config key `BAUD_RATE` needs also to be used by every device connected for sending and receiving data.

# MQTT topic hierarchy
In order to communicate with the MQTT broker in a consistent way, a topic hierarchy had to be defined. This hierarchy allows to access a fixed structure through which all necessary information can be exchanged.\
In the following examples and tables there's also defined whether the topic is/must be retained, and on which QoS-Level information should be exchanged.\
This is crucial for the gateway working correctly.

The hierarchy generally starts with the gateway ID, and can optionally be supplemented by a preﬁx (The gateway ID will only be preceeded by a slash (`/`) if a prefix is given):\
`<optionalPrefix>/<gatewayId>`

The gateway ID is then followed by a subtopic:
`<optionalPrefix>/<gatewayId>/<subtopic>`

Those subtopics can be one of the following:

| Topic | Purpose | QoS-Level | Retained? |
| ----- | ------- | --------- | --------- |
| `connected` | Shows whether the gateway is connected or not (0 or 1) | 2 | true |
| `devices` | Contains all device information & (sub-)topics | 2 | true |
| `command` | Endpoint on which the gateway listens on commands<br>Messages send to this endpoint should NOT be retained, to prevent unwanted execution of commands | 2 | false |
| `response` | Topic where possible responses get send to after a gateway command has been invoked | 2 | true |
| `devicelist` | Topic where the device list gets send to, if requested via gateway command | 2 | true |

In the case of the `devices` topic, the topic has a subtopic for each device ID which is or was connected:\
`<optionalPrefix>/<gatewayId>/devices/<deviceId>`

And then again, this topic has a subtopic for each message type the specific device has send a message with:\
`<optionalPrefix>/<gatewayId>/devices/<deviceId>/<messageType>`

Apart from those subtopics based on message types, there are two special subtopics a device can have:

| Topic | Purpose | QoS-Level | Retained? |
| ----- | ------- | --------- | --------- |
| `connected` | Shows whether the device is connected or not (0 or 1) | 2 | true |
| `command` | Endpoint on which the gateway listens on commands for serial devices<br>Messages send to this endpoint should NOT be retained, to prevent unwanted execution of commands. | 2 | false |

These two subtopics should never be used as a message type by any of the connected devices.

Every topic can be listened on, in order to -for instance- log data to a database or something similar.\
This could be easily done with a wildcard subscription: `<optionalPrefix>/<gatewayId>/#`\
Refer to MQTT's documentation in order to get more information on how to subscribe to topics in different ways.

A full topic hierarchy of a gateway could look like this:
```
gateways/
  |--->gateway1/
  |      |--->connected
  |      |--->command
  |      |--->response
  |      |--->devicelist
  |      \--->devices/
  |             |--->device1/
  |             |      |--->connected
  |             |      |--->command
  |             |      |--->id
  |             |      |--->active
  |             |      |--->info
  |             |      \--->status
  |             \--->device2/
  |                    |--->connected
  |                    |--->command
  |                    |--->id
  |                    |--->active
  |                    |--->info
  |                    \--->status
  |--->gateway2/
  ...
```
# Usage
## Configuration files
The Serial2MqttGateway can be easily configured with three different config files.

### Main Config file
This is the basic config of the gateway. It uses the [Config](https://github.com/je-s/Config) class.\
On top of the mandatory configuration keys defined [here](https://github.com/je-s/SerialPortGateway#Main-Config-file), the gateway needs following additional keys defined:

| Key | Purpose | Value Description | Default |
| --- | ------- | ----------------- | ------- |
| GATEWAY_ID | ID of the gateway<br>Needed to use and distinguish multiple gateways on the same topic level from one another<br>Is also important to be able to receive gateway commands | String | `testgateway1` |
| MQTT_PROTOCOL | Specifies the protocol to use for communication with a broker | String<br><br>- `tcp` for unencrypted MQTT/TCP communication<br>- `ssl` for encrypted MQTT/TCP communication<br>- `ws` for unencrypted websocket communication<br>- `wss` for encrypted websocket communication | `tcp`
| MQTT_CA_FILE | Configuration on whether to use a CA file or not, in case you want to use encrypted communication for MQTT | String<br><br>- `none` if no CA file is used<br>- `<path/to/caFile>` if a CA file is used | `none` |
| MQTT_KEY_FILE | Configuration on whether to use a client key file or not, in case you want to use encrypted communication for MQTT | String<br><br>- `none` if no key file is used<br>- `<path/to/keyFile>` if a key file is used | `none` |
| MQTT_HOST | MQTT host, where the broker is running | String<br><br> Hostname or IP | `test.mosquitto.org` |
| MQTT_PORT | MQTT port, where the broker is listening on | Port number | `1883` |
| MQTT_WAIT_UNTIL_RECONNECT | Time to wait until the next reconnect attempt in ms | Integer | `1000` |
| MQTT_USERNAME | MQTT username to use when logging in | String<br><br>Can be empty | `test` |
| MQTT_PASSWORD | MQTT password to use when logging in | String<br><br>Can be empty | `Test1234` |
| MQTT_TOPIC_PREFIX | MQTT topic prefix which should be used<br>Is set before every topic the Serial2MqttGateway is publishing on | String<br><br>Can be empty | `test/gateways` |

### Hardware ID Whitelist
The hardware ID whitelist lists all allowed hardware IDs;
* **If the list is empty, no whitelist-checks will be performed.**
* If the list has one or more entries, only devices matching one of those hardware IDs can/will be added to the gateway.

The hardware IDs are formatted in the following way: `<VendorID>:<ProductID>`\
The hardware IDs are separated from each other by a line break. A hardware ID whitelist can look like the following:
```
1a86:7523
0403:6001
2341:0042
```
These IDs can be retrieved with Linux tools such as "lsusb" or "udevadm".

### Serial Port Blacklist
The serial port blacklist lists all forbidden serial port paths that should be ignored when scanning for or adding new devices. The port paths are separated from each other by a line break. A port blacklist can look like the following:
```
/dev/ttyS0
/dev/ttyS1
/dev/ttyS2
/dev/ttyS3
```

## Starting the application in a Docker container
* The container needs to run in priviledged mode in order to gain access to the serial ports.
* In case you want to use the container just as your development environment, do one of the following things:
    * `docker-compose up -d devenv; docker attach serial2mqttgateway_daemon_<number>;`
    * `docker run --privileged -it serial2mqttgateway /bin/bash`
* For starting the application in a Docker container, simply use the `docker-compose.yml`-File included;\
Run `docker-compose up -d daemon` if you want to have the `serial2mqtt-gateway` run in the background.\
* In case you didn't build the image already, docker-compose will do this automatically for you.

Take a look at the [Notes](#Notes) for some restrictions when using Docker as container runtime in conjunction with serial ports.

## Communicating with the gateway
In order to send commands to the gateway, you must send a message with the corresponding content to this topic:\
`<optionalPrefix>/<gatewayId>/command`

If the command requires an argument, the argument needs to be separated by a space character:\
`<command> <argument>`

There are following commands currently implemented:

| Command | Purpose | Arguments |
| ------- | ------- | --------- |
| devicelist | Publishs a list of all connected devices on `<optionalPrefix>/<gatewayId>/devicelist` |  |
| serialportlist | Publishs a list of the system's available serial ports on `<optionalPrefix>/<gatewayId>/response` |  |
| mappinglist | Publishs a list of all mappings between connected devices by their ID & their corresponding serial port on `<optionalPrefix>/<gatewayId>/response` |  |
| broadcast | Broadcasts a message/command to all connected serial devices | `<message>` |
| add | Tries to add a serial device on a specific serial port | `<path/to/serial/port>` |
| addnew | Tries to add all new serial devices, which are not already registered/added in the gateway |  |
| delete | Tries to delete a serial device with a specific device ID | `<deviceId>` |
| deleteall | Tries to delete all currently registered serial devices from the gateway |  |

## Communicating with the devices
In order to send commands to a specific device, you must send a message with the corresponding content to this topic:\
`<optionalPrefix>/<gatewayId>/devices/<deviceId>/command`

# Notes
* In case you're using Arduinos with the Serial2MqttGateway, you can use the [ArduinoStreamCommander](https://github.com/je-s/ArduinoStreamCommander).\
But this doesn't mean you have to, as long as you follow the [requirements for communication](#requirements-for-communicating-between-the-Serial2MqttGateway-and-serial-devices).
* Docker can -without doing anything else- only provide devices or ports to the container which are already connected when the container is started. In this case this means that all serial devices to be connected must be initially available at least at the start of the container, so that the port numbers are registered in the container. Dis- and reconnects that happen afterwards -and thus the exchange of devices to other ports- are unproblematic, but to register additional devices a reboot is required.\
Meaning: If more ports should be used than when the container was initially started, the container must be restarted.

# License
```
   Copyright 2019 Jan-Eric Schober

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```
