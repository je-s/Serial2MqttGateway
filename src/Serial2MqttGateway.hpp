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
#include <thread>
#include <chrono>

// Paho MQTT CPP Library
#include <mqtt/async_client.h>

class Serial2MqttGateway : public SerialPortGateway // Inherits from SerialPortGateway
{
private:
    // Constants
    static const std::string MQTT_SSL_NO_PATH;
    static const std::string MQTT_SSL_NO_FILE;
    static const std::string MQTT_PROTOCOL_SSL;
    static const std::string MQTT_PROTOCOL_WSS;
    static const std::string MQTT_CONNECTION_SUCCESSFUL;
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

    // Variables
    std::string id;
    std::string mqttProtocol;
    std::string mqttHost;
    int mqttPort;
    int mqttWaitUntilReconnect;
    std::string mqttCertificateAuthorityPath;
    std::string mqttServerCertificateFile;
    std::string mqttClientCertificateFile;
    std::string mqttClientKeyFile;
    std::string mqttUsername;
    std::string mqttPassword;
    std::string mqttTopicPrefix;
    mqtt::connect_options mqttConnectionOptions;
    mqtt::async_client * mqttClientInstance;

    // Methods
    void setGatewayId( std::string id );
    std::string getGatewayId();
    void setMqttProtocol( std::string mqttProtocol );
    std::string getMqttProtocol();
    void setMqttHost( std::string mqttHost );
    std::string getMqttHost();
    void setMqttPort( int mqttPort );
    int getMqttPort();
    void setMqttWaitUntilReconnect( int mqttWaitUntilReconnect );
    int getMqttWaitUntilReconnect();
    std::string getMqttServerUri();
    void setMqttCertificateAuthorityPath( std::string mqttCertificateAuthorityPath );
    std::string getMqttCertificateAuthorityPath();
    void setMqttServerCertificateFile( std::string mqttServerCertificateFile );
    std::string getMqttServerCertificateFile();
    void setMqttClientCertificateFile( std::string mqttClientCertificateFile );
    std::string getMqttClientCertificateFile();
    void setMqttClientKeyFile( std::string mqttClientKeyFile );
    std::string getMqttClientKeyFile();
    void setMqttUsername( std::string mqttUsername );
    std::string getMqttUsername();
    void setMqttPassword( std::string mqttPassword );
    std::string getMqttPassword();
    void setMqttTopicPrefix( std::string mqttTopicPrefix );
    std::string getMqttTopicPrefix();
    std::string getMqttTopic( std::string appendix = "" );
    void setMqttConnectionOptions( mqtt::connect_options mqttConnectionOptions );
    mqtt::connect_options getMqttConnectionOptions();

    void setMqttClientInstance( mqtt::async_client * mqttClientInstance );
    mqtt::async_client * getMqttClientInstance();
    void loadConfig();
    void initMqtt();
    void connectToMqttBroker();
    void reconnectToMqttBroker();
    void deleteMqttInstance();

    void onMqttConnect( std::string connectionResponse );
    void onMqttDisconnect( std::string connectionResponse );
    void onMqttMessage( std::string topic, std::string command );
    bool publishMqttMessage( std::string topic, std::string message, int qos = 2, bool retain = false );
    bool subscribeToMqttTopic( std::string topic, int qos = 2 );
    bool unsubscribeFromMqttTopic( std::string topic );

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