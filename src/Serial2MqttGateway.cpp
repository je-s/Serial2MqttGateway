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

#include "Serial2MqttGateway.hpp"

const std::string Serial2MqttGateway::TOPIC_STATUS = "status";
const std::string Serial2MqttGateway::TOPIC_DEVICES = "devices";
const std::string Serial2MqttGateway::TOPIC_COMMAND = "command";
const std::string Serial2MqttGateway::TOPIC_DEVICELIST = "devicelist";
const std::string Serial2MqttGateway::TOPIC_RESPONSE = "response";
const std::string Serial2MqttGateway::TOPIC_CONNECTED = "connected";
const std::string Serial2MqttGateway::MESSAGE_CONNECTED = "1";
const std::string Serial2MqttGateway::MESSAGE_DISCONNECTED = "0";
const std::string Serial2MqttGateway::COMMAND_DELIMITER = " ";
const std::string Serial2MqttGateway::COMMAND_DEVICELIST = "devicelist";
const std::string Serial2MqttGateway::COMMAND_SERIALPORTLIST = "serialportlist";
const std::string Serial2MqttGateway::COMMAND_MAPPINGLIST = "mappinglist";
const std::string Serial2MqttGateway::COMMAND_BROADCAST = "broadcast";
const std::string Serial2MqttGateway::COMMAND_ADDDEVICE = "add";
const std::string Serial2MqttGateway::COMMAND_ADDNEWDEVICES = "addnew";
const std::string Serial2MqttGateway::COMMAND_DELETEDEVICE = "delete";
const std::string Serial2MqttGateway::COMMAND_DELETEALLDEVICES = "deleteall";

Serial2MqttGateway::Serial2MqttGateway( std::string configFile, std::string hardwareWhitelistFile, std::string serialPortBlacklistFile, std::string logPath ) : SerialPortGateway( configFile, hardwareWhitelistFile, serialPortBlacklistFile, logPath )
{
    loadConfig();
}

Serial2MqttGateway::~Serial2MqttGateway()
{
    deleteMqttInstance();
}

void Serial2MqttGateway::setGatewayId( std::string id )
{
    if ( id.empty() )
    {
        throw Exception( "Gateway ID must not be empty." );
    }

    this->id = id;
}

std::string Serial2MqttGateway::getGatewayId()
{
    return this->id;
}

void Serial2MqttGateway::setMqttHost( std::string mqttHost )
{
    if ( mqttHost.empty() )
    {
        throw Exception( "MQTT host must not be empty." );
    }

    this->mqttHost = mqttHost;
}

std::string Serial2MqttGateway::getMqttHost()
{
    return this->mqttHost;
}

void Serial2MqttGateway::setMqttPort( int mqttPort )
{
    this->mqttPort = mqttPort;
}

int Serial2MqttGateway::getMqttPort()
{
    return this->mqttPort;
}

void Serial2MqttGateway::setMqttCaFile( std::string mqttCaFile )
{
    this->mqttCaFile = mqttCaFile;
}

std::string Serial2MqttGateway::getMqttCaFile()
{
    return this->mqttCaFile;
}

void Serial2MqttGateway::setMqttUsername( std::string mqttUsername )
{
    this->mqttUsername = mqttUsername;
}

std::string Serial2MqttGateway::getMqttUsername()
{
    return this->mqttUsername;
}

void Serial2MqttGateway::setMqttPassword( std::string mqttPassword )
{
    this->mqttPassword = mqttPassword;
}

std::string Serial2MqttGateway::getMqttPassword()
{
    return this->mqttPassword;
}

void Serial2MqttGateway::setMqttTopicPrefix( std::string mqttTopicPrefix )
{
    this->mqttTopicPrefix = mqttTopicPrefix;
}

std::string Serial2MqttGateway::getMqttTopicPrefix()
{
    return this->mqttTopicPrefix;
}

std::string Serial2MqttGateway::getMqttTopic( std::string appendix )
{
    std::stringstream topic;

    if ( !getMqttTopicPrefix().empty() )
    {
        topic << getMqttTopicPrefix() << "/";
    }

    topic << getGatewayId();

    if ( !appendix.empty() )
    {
        topic << appendix;
    }

    return topic.str();
}

void Serial2MqttGateway::loadConfig()
{
    Config * config = getConfigInstance();

    std::string id = config->getString( "GATEWAY_ID" );
    std::string mqttCaFile = config->getString( "MQTT_CAFILE" );
    std::string mqttHost = config->getString( "MQTT_HOST" );
    int mqttPort = config->getInteger( "MQTT_PORT" );
    std::string mqttUsername = config->getString( "MQTT_USERNAME" );
    std::string mqttPassword = config->getString( "MQTT_PASSWORD" );
    std::string mqttTopicPrefix = config->getString( "MQTT_TOPIC_PREFIX" );

    setGatewayId( id );
    setMqttCaFile( mqttCaFile );
    setMqttHost( mqttHost );
    setMqttPort( mqttPort );
    setMqttUsername( mqttUsername );
    setMqttPassword( mqttPassword );
    setMqttTopicPrefix( mqttTopicPrefix );
}

void Serial2MqttGateway::initMqtt()
{
    std::string caFile = getMqttCaFile();

    mosqpp::lib_init();

    if ( caFile != "none" )
    {
        mosqpp::mosquittopp::tls_set( caFile.c_str() );
    }

    mosqpp::mosquittopp::username_pw_set( getMqttUsername().c_str(), getMqttPassword().c_str() );
    mosqpp::mosquittopp::will_set( getMqttTopic( "/" + TOPIC_CONNECTED ).c_str(), MESSAGE_DISCONNECTED.length(), MESSAGE_DISCONNECTED.c_str(), 2, true );
    mosqpp::mosquittopp::connect_async( getMqttHost().c_str(), getMqttPort(), 60 );
    mosqpp::mosquittopp::loop_start();
}

void Serial2MqttGateway::deleteMqttInstance()
{
    mosqpp::mosquittopp::loop_stop();
    mosqpp::lib_cleanup();
}

void Serial2MqttGateway::onMqttConnect( int connectionResponse )
{
    if ( connectionResponse == MQTT_CONNECTION_SUCCESSFUL )
    {
        getLoggerInstance()->writeInfo( std::string( "Connected to MQTT-Broker \"" + getMqttHost() + ":" + std::to_string( getMqttPort() ) + "\"." ) );
        publishMqttMessage( getMqttTopic( "/" + TOPIC_CONNECTED ), MESSAGE_CONNECTED, 2, true );
        subscribeToMqttTopic( getMqttTopic( "/" + TOPIC_COMMAND ), 2 );
        subscribeToMqttTopic( getMqttTopic( "/" + TOPIC_DEVICES + "/+/" + TOPIC_COMMAND ), 2 );
    }
    else
    {
        getLoggerInstance()->writeError( std::string( "Couldn't connect to MQTT-Broker \"" + getMqttHost() + ":" + std::to_string( getMqttPort() ) + "\"." ) );
    }
}

void Serial2MqttGateway::onMqttDisconnect( int connectionResponse )
{
    getLoggerInstance()->writeError( std::string( "Disconnected from MQTT-Broker: \"" + getMqttHost() + ":" + std::to_string( getMqttPort() ) + "\"." ) );
    getLoggerInstance()->writeInfo( std::string( "Trying to reconnect to MQTT-Broker \"" + getMqttHost() + ":" + std::to_string( getMqttPort() ) + "\"." ) );
    mosqpp::mosquittopp::reconnect_async();
}

void Serial2MqttGateway::onMqttMessage( std::string topic, std::string message )
{
    // Expression extracts ids from topics like:
    //  /<prefix>/gateways/<gatewayId>/command -> <gatewayId>
    //  /<prefix>/gateways/<gatewayId>/devices/<deviceId>/command -> <deviceId>
    std::regex matchIdExpression( "(?!.*(\\/.*\\/)).*(?=\\/" + TOPIC_COMMAND + ")" );

    // Expression extracts parent topics from topics like:
    //  /<prefix>/gateways/<gatewayId>/command -> gateways
    //  /<prefix>/gateways/<gatewayId>/devices/<deviceId>/command -> devices
    std::regex matchParentTopicExpression( "(?:.*\\/|)(.*)(?=\\/.*\\/" + TOPIC_COMMAND + ")" );

    std::smatch matchId, matchParentTopic;
    std::regex_search( topic, matchId, matchIdExpression );
    std::regex_search( topic, matchParentTopic, matchParentTopicExpression );
    std::string id = matchId.str(); // Can bei either a deviceId or a gatewayId; depends on the context
    std::string parentTopic = matchParentTopic[1].str(); // Extract Capture-Group 1, not the whole match

    if ( parentTopic == TOPIC_DEVICES )
    {
        if ( id.empty() )
        {
            getLoggerInstance()->writeWarn( std::string( "Could not deliver message \"" + message + "\", since device ID on topic \"" + topic + "\" was empty." ) );

            return;
        }

        if ( message.empty() )
        {
            getLoggerInstance()->writeWarn( std::string( "Can not deliver empty message to device with ID \"" + id + "\"." ) );

            return;
        }

        sendMessageToSerialDevice( id, message );
    }
    else
    {
        if ( message.empty() )
        {
            getLoggerInstance()->writeWarn( std::string( "Can not deliver empty message to the local gateway with ID \"" + id + "\"." ) );

            return;
        }

        processGatewayCommand( message );
    }
}

void Serial2MqttGateway::publishMqttMessage( std::string topic, std::string message, int qos, bool retain )
{
    mosqpp::mosquittopp::publish(
        NULL,
        topic.c_str(),
        message.length(),
        message.c_str(),
        qos,
        retain
    );
}

void Serial2MqttGateway::subscribeToMqttTopic( std::string topic, int qos )
{
    mosqpp::mosquittopp::subscribe(
        NULL,
        topic.c_str(),
        qos
    );
}

void Serial2MqttGateway::unsubscribeFromMqttTopic( std::string topic )
{
    mosqpp::mosquittopp::unsubscribe(
        NULL,
        topic.c_str()
    );
}

void Serial2MqttGateway::publishSerialMessageToMqtt( SerialMessage serialMessage )
{
    std::stringstream topic;
    topic << getMqttTopic() << "/" << TOPIC_DEVICES << "/" << serialMessage.getDeviceId() << "/" << serialMessage.getType();

    std::stringstream jsonMessage;
    jsonMessage
        << "{"
        << "\"timestamp\":\"" << serialMessage.getTimestamp() << "\","
        << "\"content\":\"" << serialMessage.getContent() << "\""
        << "}";

    publishMqttMessage( topic.str(), jsonMessage.str(), 2, true );
}

void Serial2MqttGateway::processGatewayCommand( std::string command )
{
    std::size_t delimiterPos = command.find_first_of( COMMAND_DELIMITER );
    std::size_t commandEnd = command.length() - 1;
    std::string arguments = "";

    // Split the command into command and arguments
    if ( commandEnd != std::string::npos )
    {
        if ( delimiterPos != std::string::npos && delimiterPos < commandEnd )
        {
            arguments = command.substr( delimiterPos + 1, commandEnd - delimiterPos ); // Also remove the newline character from the content
        }

        command = command.substr( 0, delimiterPos );
    }

    getLoggerInstance()->writeInfo( std::string( "-> Received gateway command \"" + command + "\" with arguments \"" + arguments + "\"." ) );

    // Try and execute the actual command with optional arguments
    executeGatewayCommand( command, arguments );
}

void Serial2MqttGateway::executeGatewayCommand( std::string command, std::string arguments )
{
    if ( command == COMMAND_DEVICELIST )
    {
        commandDeviceList();
    }
    else if ( command == COMMAND_SERIALPORTLIST )
    {
        commandSerialPortList();
    }
    else if ( command == COMMAND_MAPPINGLIST )
    {
        commandDeviceIdToSerialPortMappingList();
    }
    else if ( command == COMMAND_BROADCAST )
    {
        commandBroadcast( arguments );
    }
    else if ( command == COMMAND_ADDDEVICE )
    {
        commandAddDevice( arguments );
    }
    else if ( command == COMMAND_ADDNEWDEVICES )
    {
        commandAddNewDevices();
    }
    else if ( command == COMMAND_DELETEDEVICE )
    {
        commandDeleteDevice( arguments );
    }
    else if ( command == COMMAND_DELETEALLDEVICES )
    {
        commandDeleteAllDevices();
    }
    else
    {
        std::string response = "Unknown gateway command \"" + command + "\".";

        getLoggerInstance()->writeWarn( "-> " + response );

        publishMqttMessage( getMqttTopic( "/" + TOPIC_RESPONSE ), response, 2, true );
    }
}

void Serial2MqttGateway::commandDeviceList()
{
    std::string deviceIdList = getDeviceIdList();

    getLoggerInstance()->writeInfo( std::string( "-> Publishing list of device IDs to MQTT; \"" + deviceIdList + "\"." ) );

    publishMqttMessage( getMqttTopic( "/" + TOPIC_DEVICELIST ), deviceIdList, 2, true );
}

void Serial2MqttGateway::commandSerialPortList()
{
    std::string serialPortList = getSerialPortList();

    getLoggerInstance()->writeInfo( std::string( "-> Publishing list of serial ports to MQTT; \"" + serialPortList + "\"." ) );

    publishMqttMessage( getMqttTopic( "/" + TOPIC_RESPONSE ), serialPortList, 2, true );
}

void Serial2MqttGateway::commandDeviceIdToSerialPortMappingList()
{
    std::string deviceIdToSerialPortMappingList = getDeviceIdToSerialPortMappingList();

    getLoggerInstance()->writeInfo( std::string( "-> Publishing list of mappings from device IDs to serial ports to MQTT; \"" + deviceIdToSerialPortMappingList + "\"." ) );

    publishMqttMessage( getMqttTopic( "/" + TOPIC_RESPONSE ), deviceIdToSerialPortMappingList, 2, true );
}

void Serial2MqttGateway::commandBroadcast( std::string message )
{
    getLoggerInstance()->writeInfo( std::string( "-> Broadcasting message \"" + message + "\" to all devices." ) );

    broadcastMessageToSerialDevices( message );
}

void Serial2MqttGateway::commandAddDevice( std::string serialPort )
{
    getLoggerInstance()->writeInfo( std::string( "-> Trying to add device on port \"" + serialPort + "\"." ) );

    bool success = addSerialDevice( serialPort );

    if ( !success )
    {
        std::string response = "Serial Device on port \"" + serialPort + "\" could not be added. Refer to the Logs for further details.";

        getLoggerInstance()->writeWarn( "-> " + response );

        publishMqttMessage( getMqttTopic( "/" + TOPIC_RESPONSE ), response, 2, true );
    }
}

void Serial2MqttGateway::commandAddNewDevices()
{
    getLoggerInstance()->writeInfo( std::string( "-> Adding new serial devices." ) );

    unsigned int numDevicesAdded = addNewSerialPorts();

    std::string response = std::to_string ( numDevicesAdded ) + " serial devices were added.";

    getLoggerInstance()->writeWarn( "-> " + response );

    publishMqttMessage( getMqttTopic( "/" + TOPIC_RESPONSE ), response, 2, true );
}

void Serial2MqttGateway::commandDeleteDevice( std::string deviceId )
{
    getLoggerInstance()->writeInfo( std::string( "-> Trying to delete device with ID \"" + deviceId + "\"." ) );

    bool success = deleteSerialDevice( deviceId );

    if ( !success )
    {
        std::string response = "Serial device with ID \"" + deviceId + "\" could not be deleted. Refer to the Logs for further details.";

        getLoggerInstance()->writeWarn( "-> " + response );

        publishMqttMessage( getMqttTopic( "/" + TOPIC_RESPONSE ), response, 2, true );
    }
}

void Serial2MqttGateway::commandDeleteAllDevices()
{
    getLoggerInstance()->writeInfo( std::string( "-> Deleting all serial devices." ) );

    unsigned int numDevicesDeleteed = deleteAllSerialDevices();

    std::string response = std::to_string ( numDevicesDeleteed ) + " serial devices were deleted.";

    getLoggerInstance()->writeWarn( "-> " + response );

    publishMqttMessage( getMqttTopic( "/" + TOPIC_RESPONSE ), response, 2, true );
}

void Serial2MqttGateway::on_connect( int connectionResponse )
{
    onMqttConnect( connectionResponse );
}

void Serial2MqttGateway::on_disconnect( int connectionResponse )
{
    onMqttDisconnect( connectionResponse );
}

void Serial2MqttGateway::on_message( const struct mosquitto_message * mqttMessage )
{
    std::string topic = std::string( mqttMessage->topic );
    std::string message( (const char *)mqttMessage->payload, mqttMessage->payloadlen );

    onMqttMessage( topic, message );
}

void Serial2MqttGateway::start()
{
    initMqtt();
    SerialPortGateway::start();
}

void Serial2MqttGateway::stop()
{
    SerialPortGateway::stop();
    deleteMqttInstance();
}

void Serial2MqttGateway::serialDeviceAddedCallback( std::string deviceId, std::string serialPort )
{
    SerialMessage serialMessage = SerialMessage( deviceId, TOPIC_CONNECTED, MESSAGE_CONNECTED );

    publishSerialMessageToMqtt( serialMessage );
}

void Serial2MqttGateway::serialDeviceDeletedCallback( std::string deviceId, std::string serialPort )
{
    SerialMessage serialMessage = SerialMessage( deviceId, TOPIC_CONNECTED, MESSAGE_DISCONNECTED );

    publishSerialMessageToMqtt( serialMessage );
}

void Serial2MqttGateway::messageCallback( SerialMessage serialMessage )
{
    // Write the the message to the log
    std::stringstream message;
    message
        << "New message from \"" << serialMessage.getDeviceId()
        << "\" on channel \"" << serialMessage.getType()
        << "\": timestamp=\"" << serialMessage.getTimestamp()
        << "\", content=\"" << serialMessage.getContent()
        << "\".";

    getLoggerInstance()->writeInfo( message.str() );

    // Publish the message over MQTT
    publishSerialMessageToMqtt( serialMessage );
}
