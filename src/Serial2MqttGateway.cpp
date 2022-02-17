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

const std::string Serial2MqttGateway::MQTT_SSL_NO_FILE = "none";
const std::string Serial2MqttGateway::MQTT_PROTOCOL_SSL = "ssl";
const std::string Serial2MqttGateway::MQTT_PROTOCOL_WSS = "wss";
const std::string Serial2MqttGateway::MQTT_CONNECTION_SUCCESSFUL = "0";
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

void Serial2MqttGateway::setMqttProtocol( std::string mqttProtocol )
{
    if ( mqttProtocol.empty() )
    {
        throw Exception( "MQTT protocol must not be empty." );
    }

    this->mqttProtocol = mqttProtocol;
}

std::string Serial2MqttGateway::getMqttProtocol()
{
    return this->mqttProtocol;
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

void Serial2MqttGateway::setMqttWaitUntilReconnect( int mqttWaitUntilReconnect )
{
    this->mqttWaitUntilReconnect = mqttWaitUntilReconnect;
}

int Serial2MqttGateway::getMqttWaitUntilReconnect()
{
    return this->mqttWaitUntilReconnect;
}

std::string Serial2MqttGateway::getMqttServerUri()
{
    return std::string( getMqttProtocol() + "://" + getMqttHost() + ":" + std::to_string( getMqttPort() ) );
}

void Serial2MqttGateway::setMqttServerCertificateFile( std::string mqttServerCertificateFile )
{
    this->mqttServerCertificateFile = mqttServerCertificateFile;
}

std::string Serial2MqttGateway::getMqttServerCertificateFile()
{
    return this->mqttServerCertificateFile;
}

void Serial2MqttGateway::setMqttClientCertificateFile( std::string mqttClientCertificateFile )
{
    this->mqttClientCertificateFile = mqttClientCertificateFile;
}

std::string Serial2MqttGateway::getMqttClientCertificateFile()
{
    return this->mqttClientCertificateFile;
}

void Serial2MqttGateway::setMqttClientKeyFile( std::string mqttClientKeyFile )
{
    this->mqttClientKeyFile = mqttClientKeyFile;
}

std::string Serial2MqttGateway::getMqttClientKeyFile()
{
    return this->mqttClientKeyFile;
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

void Serial2MqttGateway::setMqttConnectionOptions( mqtt::connect_options mqttConnectionOptions )
{
    this->mqttConnectionOptions = mqttConnectionOptions;
}

mqtt::connect_options Serial2MqttGateway::getMqttConnectionOptions()
{
    return this->mqttConnectionOptions;
}

void Serial2MqttGateway::loadConfig()
{
    Config * config = getConfigInstance();

    std::string id = config->getString( "GATEWAY_ID" );
    std::string mqttProtocol = config->getString( "MQTT_PROTOCOL" );
    std::string mqttHost = config->getString( "MQTT_HOST" );
    int mqttPort = config->getInteger( "MQTT_PORT" );
    int mqttWaitUntilReconnect = config->getInteger( "MQTT_WAIT_UNTIL_RECONNECT" );
    std::string mqttServerCertificateFile = config->getString( "MQTT_SERVER_CERTIFICATE_FILE" );
    std::string mqttClientCertificateFile = config->getString( "MQTT_CLIENT_CERTIFICATE_FILE" );
    std::string mqttClientKeyFile = config->getString( "MQTT_CLIENT_KEY_FILE" );
    std::string mqttUsername = config->getString( "MQTT_USERNAME" );
    std::string mqttPassword = config->getString( "MQTT_PASSWORD" );
    std::string mqttTopicPrefix = config->getString( "MQTT_TOPIC_PREFIX" );

    setGatewayId( id );
    setMqttProtocol( mqttProtocol );
    setMqttHost( mqttHost );
    setMqttPort( mqttPort );
    setMqttWaitUntilReconnect( mqttWaitUntilReconnect );
    setMqttServerCertificateFile( mqttServerCertificateFile );
    setMqttClientCertificateFile( mqttClientCertificateFile );
    setMqttClientKeyFile( mqttClientKeyFile );
    setMqttUsername( mqttUsername );
    setMqttPassword( mqttPassword );
    setMqttTopicPrefix( mqttTopicPrefix );
}

void Serial2MqttGateway::setMqttClientInstance( mqtt::async_client * mqttClientInstance )
{
    this->mqttClientInstance = mqttClientInstance;
}

mqtt::async_client * Serial2MqttGateway::getMqttClientInstance()
{
    return this->mqttClientInstance;
}

void Serial2MqttGateway::initMqtt()
{
    mqtt::connect_options mqttConnectionOptions = mqtt::connect_options();
    std::string protocol = getMqttProtocol();

    if ( protocol == MQTT_PROTOCOL_SSL || protocol == MQTT_PROTOCOL_WSS )
    {
        getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: Initialising for transport protocol '" + protocol + "'." ) );
        mqtt::ssl_options sslOptions = mqtt::ssl_options();

        std::string mqttServerCertificateFile = getMqttServerCertificateFile();
        std::string mqttClientCertificateFile = getMqttClientCertificateFile();
        std::string mqttClientKeyFile = getMqttClientKeyFile();

        if ( mqttServerCertificateFile != MQTT_SSL_NO_FILE )
        {
            sslOptions.set_trust_store( mqttServerCertificateFile );
            getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: Loaded Server Certificate file from '" + mqttServerCertificateFile + "'." ) );
        }
        else
        {
            getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: No Server Certificate file given." ) );
        }

        if ( mqttClientCertificateFile != MQTT_SSL_NO_FILE )
        {
            sslOptions.set_key_store( mqttClientCertificateFile );
            getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: Loaded Client Certificate file from '" + mqttClientCertificateFile + "'." ) );
        }
        else
        {
            getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: No Client Certificate file given." ) );
        }

        if ( mqttClientKeyFile != MQTT_SSL_NO_FILE )
        {
            sslOptions.set_private_key( mqttClientKeyFile );
            getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: Loaded Client Key file from '" + mqttClientKeyFile + "'." ) );
        }
        else
        {
            getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: No Client Key file given." ) );
        }

        sslOptions.set_error_handler
        (
            [this]( const std::string & msg )
            {
                getLoggerInstance()->writeError( std::string( "MQTT: SSL Error: " + msg ) );
            }
        );

        mqttConnectionOptions.set_ssl( std::move( sslOptions ) );
    }
    else
    {
        getLoggerInstance()->writeInfo( std::string( "MQTT: SSL: No secure communication initialised for transport protocol '" + protocol + "'." ) );
    }

    mqttConnectionOptions.set_user_name( getMqttUsername() );
    mqttConnectionOptions.set_password( getMqttPassword() );
    mqttConnectionOptions.set_will( mqtt::will_options( getMqttTopic( "/" + TOPIC_CONNECTED ), MESSAGE_DISCONNECTED, 2, true ) );
    setMqttConnectionOptions( mqttConnectionOptions );

    mqtt::async_client * mqttClientInstance = new mqtt::async_client( getMqttServerUri(), getGatewayId() );

    mqttClientInstance->set_connected_handler
    (
        [this]( const std::string & connectionResponse )
        {
            onMqttConnect( connectionResponse );
        }
    );

    mqttClientInstance->set_connection_lost_handler
    (
        [this]( const std::string & connectionResponse )
        {
            onMqttDisconnect( connectionResponse );
        }
    );

    mqttClientInstance->set_message_callback
    (
        [this]( mqtt::const_message_ptr mqttMessage )
        {
            std::string topic = mqttMessage->get_topic();
            std::string message = mqttMessage->to_string();

            onMqttMessage( topic, message );
        }
    );

    setMqttClientInstance( mqttClientInstance );
    connectToMqttBroker();
}

void Serial2MqttGateway::connectToMqttBroker()
{
    try
    {
        getLoggerInstance()->writeInfo( std::string( "MQTT: Connecting to MQTT-Broker \"" + getMqttServerUri() + "\"." ) );
        getLoggerInstance()->writeInfo( std::string( "MQTT: Waiting for connection to MQTT-Broker..." ) );

        getMqttClientInstance()->connect()->wait();
    }
    catch ( const mqtt::exception & e )
    {
        getLoggerInstance()->writeError( std::string( "MQTT: Couldn't connect to MQTT-Broker. Reason code: \"" + std::to_string( e.get_reason_code() ) + "\", message: \"" + e.get_message() + "\". Retrying... " ) );
        std::this_thread::sleep_for( std::chrono::milliseconds( getMqttWaitUntilReconnect() ) );
        std::thread( &Serial2MqttGateway::connectToMqttBroker, this ).detach();
    }
}

void Serial2MqttGateway::reconnectToMqttBroker()
{
    getLoggerInstance()->writeInfo( std::string( "MQTT: Trying to reconnect to MQTT-Broker \"" + getMqttServerUri() + "\"." ) );

    try
    {
        getMqttClientInstance()->reconnect();
    }
    catch ( const mqtt::exception & e )
    {
        getLoggerInstance()->writeError( std::string( "MQTT: Error while reconnecting to MQTT-Broker. Reason code: \"" + std::to_string( e.get_reason_code() ) + "\", message: \"" + e.get_message() + "\". Retrying... " ) );
        std::this_thread::sleep_for( std::chrono::milliseconds( getMqttWaitUntilReconnect() ) );
        std::thread( &Serial2MqttGateway::reconnectToMqttBroker, this ).detach();
    }
}

void Serial2MqttGateway::deleteMqttInstance()
{
    try
    {
        getLoggerInstance()->writeInfo( std::string( "MQTT: Disconnecting from MQTT-Broker \"" + getMqttServerUri() + "\"." ) );
        getMqttClientInstance()->disconnect()->wait();
        getLoggerInstance()->writeInfo( std::string( "MQTT: Disconnected." ) );
    }
    catch ( const mqtt::exception & e )
    {
        getLoggerInstance()->writeError( std::string( "MQTT: Error while disconnecting from MQTT-Broker. Reason code: \"" + std::to_string( e.get_reason_code() ) + "\", message: \"" + e.get_message() + "\". Retrying... " ) );
        std::thread( &Serial2MqttGateway::deleteMqttInstance, this ).detach();
    }

    delete mqttClientInstance;
}

void Serial2MqttGateway::onMqttConnect( std::string connectionResponse )
{
    getLoggerInstance()->writeInfo( std::string( "MQTT: Connected to MQTT-Broker \"" + getMqttServerUri() + "\". Response code: \"" + connectionResponse + "\"." ) );

    publishMqttMessage( getMqttTopic( "/" + TOPIC_CONNECTED ), MESSAGE_CONNECTED, 2, true );
    subscribeToMqttTopic( getMqttTopic( "/" + TOPIC_COMMAND ), 2 );
    subscribeToMqttTopic( getMqttTopic( "/" + TOPIC_DEVICES + "/+/" + TOPIC_COMMAND ), 2 );

}

void Serial2MqttGateway::onMqttDisconnect( std::string connectionResponse )
{
    getLoggerInstance()->writeError( std::string( "MQTT: Disconnected from MQTT-Broker: \"" + getMqttServerUri() + "\". Response code: \"" + connectionResponse + "\"." ) );
    reconnectToMqttBroker();
}

void Serial2MqttGateway::onMqttMessage( std::string topic, std::string message )
{
    getLoggerInstance()->writeInfo( std::string( "MQTT: Received message on topic \"" + topic + "\": \"" + message + "\"." ) );

    // Expression extracts ids from topics like:
    //  /<prefix>/gateways/<gatewayId>/command -> <gatewayId>
    //  /<prefix>/gateways/<gatewayId>/devices/<deviceId>/command -> <deviceId>
    std::regex matchIdExpression( "(?!.*(\\/.*\\/)).*(?=\\/" + TOPIC_COMMAND + ")" );

    // Expression extracts parent topics from topics like:
    //  /<prefix>/gateways/<gatewayId>/command -> gateways
    //  /<prefix>/gateways/<gatewayId>/devices/<deviceId>/command -> devices
    // Based on that we can determine if as message is destined for the gateway itself or a specific device
    // This logic can be easily extended to incorporate actions on other branches, on the same level of "devices"
    std::regex matchParentTopicExpression( "(?:.*\\/|)(.*)(?=\\/.*\\/" + TOPIC_COMMAND + ")" );

    std::smatch matchId, matchParentTopic;
    std::regex_search( topic, matchId, matchIdExpression );
    std::regex_search( topic, matchParentTopic, matchParentTopicExpression );
    std::string id = matchId.str(); // Can be either a deviceId or a gatewayId; depends on the context
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

bool Serial2MqttGateway::publishMqttMessage( std::string topic, std::string message, int qos, bool retain )
{
    try
    {
        getMqttClientInstance()->publish( topic, message, qos, retain );

        return true;
    }
    catch ( const mqtt::exception & e )
    {
        getLoggerInstance()->writeError( std::string( "MQTT: Error while publishing message. Reason code: \"" + std::to_string( e.get_reason_code() ) + "\", message: \"" + e.get_message() + "\"." ) );

        return false;
    }
}

bool Serial2MqttGateway::subscribeToMqttTopic( std::string topic, int qos )
{
    try
    {
        getMqttClientInstance()->subscribe( topic, qos );

        return true;
    }
    catch ( const mqtt::exception & e )
    {
        getLoggerInstance()->writeError( std::string( "MQTT: Error while sending subscribing to topic. Reason code: \"" + std::to_string( e.get_reason_code() ) + "\", message: \"" + e.get_message() + "\"." ) );

        return false;
    }
}

bool Serial2MqttGateway::unsubscribeFromMqttTopic( std::string topic )
{
    try
    {
        getMqttClientInstance()->unsubscribe( topic );

        return true;
    }
    catch ( const mqtt::exception & e )
    {
        getLoggerInstance()->writeError( std::string( "MQTT: Error while unsubscribing from topic. Reason code: \"" + std::to_string( e.get_reason_code() ) + "\", message: \"" + e.get_message() + "\"." ) );

        return false;
    }
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
