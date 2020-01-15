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

// C Standard Libraries
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// C++ Standard Libraries
#include <iostream>
#include <stdexcept>
#include <thread>

// SerialPortGateway Class
#include "Serial2MqttGateway.hpp"

Serial2MqttGateway * gateway = nullptr;
bool gatewayStarted = true;

void stopGateway( int param )
{
    std::cout << std::endl << "Stopping Gateway..." << std::endl;

    gatewayStarted = false;
    gateway->stop();
    
    while( !gateway->isEveryReadLoopQuitted() )
    {
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    }

    std::cout << "Gateway stopped." << std::endl;
    exit( 0 );
}

int main( int argc, char* argv[] )
{
    signal( SIGINT, stopGateway );

    try
    {
        if ( argc < 5 )
        {
            throw std::invalid_argument( "Not all start parameters given. Format: \"" + std::string( argv[0] ) + " <configFile> <hardwareWhitelistFile> <serialPortBlacklistFile> <logPath>\"." );
        }

        std::string configFile = argv[1];
        std::string hardwareWhitelistFile = argv[2];
        std::string serialPortBlacklistFile = argv[3];
        std::string logPath = argv[4];

        gateway = new Serial2MqttGateway( configFile, hardwareWhitelistFile, serialPortBlacklistFile, logPath );

        std::cout << "Starting Gateway.." << std::endl;
        gateway->start();
        std::cout << "Gateway started." << std::endl;
        std::cout << "CTRL+C to exit." << std::endl;

        // Infinite Loop
        while ( gatewayStarted )
        {
            std::this_thread::sleep_for( std::chrono::seconds( 10 ) );
        }
    }
    catch ( ConfigMalformedException & e )
    {
        std::cerr << "ConfigMalformedException: " << e.what() << std::endl;

        return 1;
    }
    catch ( ConfigMissingException & e )
    {
        std::cerr << "ConfigMissingException: " << e.what() << std::endl;

        return 1;
    }
    catch ( ConfigKeyNotFoundException & e )
    {
        std::cerr << "ConfigKeyNotFoundException: " << e.what() << std::endl;

        return 1;
    }
    catch ( NumericConfigValueMalformedException & e )
    {
        std::cerr << "NumericConfigValueMalformedException: " << e.what() << std::endl;

        return 1;
    }
    catch ( serial::IOException & e )
    {
        std::cerr << "IOException: " << e.what() << std::endl;

        return 1;
    }
    catch ( serial::SerialException & e )
    {
        std::cerr << "SerialException: " << e.what() << std::endl;

        return 1;
    }
    catch ( serial::PortNotOpenedException & e )
    {
        std::cerr << "PortNotOpenedException: " << e.what() << std::endl;

        return 1;
    }
    catch ( Exception & e )
    {
        std::cerr << "Exception: " << e.what() << std::endl;

        return 1;
    }
    catch ( std::exception & e )
    {
        std::cerr << "exception: " << e.what() << std::endl;

        return 1;
    }

    return 0;
}
