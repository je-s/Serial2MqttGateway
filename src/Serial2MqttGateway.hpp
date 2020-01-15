/*
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
*/

#ifndef SERIAL2MQTTGATEWAY_HPP
#define SERIAL2MQTTGATEWAY_HPP

// SerialPortGateway Class
#include "../dependencies/SerialPortGateway/src/SerialPortGateway.hpp"

// C++ Standard Libraries
#include <regex>

// Mosquitto Library
#include <mosquittopp.h>

class Serial2MqttGateway : public SerialPortGateway, public mosqpp::mosquittopp // Inherits from both SerialPortGateway and mosquittopp
{
private:
    // Constants
    static const std::string TOPIC_STATUS;
    static const std::string TOPIC_DEVICES;
    static const std::string TOPIC_COMMAND;
    static const std::string TOPIC_DEVICELIST;
    static const std::string TOPIC_RESPONSE;
    static const std::string TOPIC_CONNECTED;
    static const std::string MESSAGE_CONNECTED;
    static const std::string MESSAGE_DISCONNECTED;
    static const std::string COMMAND_DELIMITER;
    static const std::string COMMAND_DEVICELIST;
    static const std::string COMMAND_SERIALPORTLIST;
    static const std::string COMMAND_MAPPINGLIST;
    static const std::string COMMAND_BROADCAST;
    static const std::string COMMAND_ADDDEVICE;
    static const std::string COMMAND_ADDNEWDEVICES;
    static const std::string COMMAND_DELETEDEVICE;
    static const std::string COMMAND_DELETEALLDEVICES;
    static const int MQTT_CONNECTION_SUCCESSFUL = 0;

    // Variables
    std::string id;
    std::string mqttCaFile;
    std::string mqttHost;
    int mqttPort;
    std::string mqttUsername;
    std::string mqttPassword;
    std::string mqttTopicPrefix;

    // Methods
    void setGatewayId( std::string id );
    std::string getGatewayId();
    void setMqttHost( std::string mqttHost );
    std::string getMqttHost();
    void setMqttPort( int mqttPort );
    int getMqttPort();
    void setMqttCaFile( std::string mqttCaFile );
    std::string getMqttCaFile();
    void setMqttUsername( std::string mqttUsername );
    std::string getMqttUsername();
    void setMqttPassword( std::string mqttPassword );
    std::string getMqttPassword();
    void setMqttTopicPrefix( std::string mqttTopicPrefix );
    std::string getMqttTopicPrefix();
    std::string getMqttTopic( std::string appendix = "" );

    void loadConfig();
    void initMqtt();
    void deleteMqttInstance();

    void onMqttConnect( int connectionResponse );
    void onMqttDisconnect( int connectionResponse );
    void onMqttMessage( std::string topic, std::string command );
    void publishMqttMessage( std::string topic, std::string message, int qos = 2, bool retain = false );
    void subscribeToMqttTopic( std::string topic, int qos = 2 );
    void unsubscribeFromMqttTopic( std::string topic );

    void publishSerialMessageToMqtt( SerialMessage serialMessage );
    void processGatewayCommand( std::string command );
    void executeGatewayCommand( std::string command, std::string arguments = "" );
    void commandDeviceList();
    void commandSerialPortList();
    void commandDeviceIdToSerialPortMappingList();
    void commandBroadcast( std::string command );
    void commandAddDevice( std::string serialPort );
    void commandAddNewDevices();
    void commandDeleteDevice( std::string deviceId );
    void commandDeleteAllDevices();

    // Private Methods - Mosquitto-Internal MQTT-Callbacks
    void on_connect( int connectionResponse );
    void on_disconnect( int connectionResponse );
    void on_message( const struct mosquitto_message *message );

public:
    // Constructors
    Serial2MqttGateway( std::string configFile, std::string hardwareWhitelistFile, std::string serialPortBlacklistFile, std::string logPath = "./logs" );

    // Destructors
    ~Serial2MqttGateway();

    // Methods
    void start();
    void stop();
    void serialDeviceAddedCallback( std::string deviceId, std::string serialPort );
    void serialDeviceDeletedCallback( std::string deviceId, std::string serialPort );
    void messageCallback( SerialMessage serialMessage );
};

#endif // SERIAL2MQTTGATEWAY_HPP