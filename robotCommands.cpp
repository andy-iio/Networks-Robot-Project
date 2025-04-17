#include "robotCommands.h"
#include "packet.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <vector>

//sends a drive packet with direction duration and speed to the robot
void sendDrivePacket(MySocket& robotSocket, unsigned short packetCount, unsigned char direction, unsigned char duration, unsigned char speed) {

    std::cout << "--- DRIVE PACKET ---" << std::endl;

    //1 set up packet
    PktDef driveCommand;
    driveCommand.setCommand(PktDef::DRIVE);
    driveCommand.SetPktCount(packetCount);

    //set drive parameters from function arguments
    struct DriveBodyData {
        unsigned char direction;
        unsigned char duration;
        unsigned char speed;
    } driveData = { direction, duration, speed };

    //2 set body
    driveCommand.SetBodyData(reinterpret_cast<char*>(&driveData), sizeof(driveData));

    //debugging
    std::cout << "Drive Data - Direction: " << static_cast<int>(driveData.direction)
        << ", Duration: " << static_cast<int>(driveData.duration)
        << ", Speed: " << static_cast<int>(driveData.speed) << std::endl;
    std::cout << "Sizeof driveData: " << sizeof(driveData) << std::endl;
    std::cout << "Calculated Packet Length: " << (int)driveCommand.GetLength() << std::endl;

    //3 calculate crc
    driveCommand.CalcCRC();
    std::cout << "Drive CRC: " << static_cast<int>(driveCommand.getCRC()) << std::endl;

    //4 generate the raw packet data
    char* rawData = driveCommand.GenPacket();
    char dataLength = driveCommand.GetLength();

    //debug prints
    std::cout << "--- Sending DRIVE Packet ---" << std::endl;
    driveCommand.printPacketDetails();
    driveCommand.printBuffer(rawData, dataLength);
    std::cout << "Sending Data Length: " << static_cast<int>(dataLength) << std::endl;

    //5 send the command to the robot
    std::cout << "Sending drive packet..." << std::endl;
    robotSocket.SendData(rawData, dataLength);
    std::cout << "Sent drive packet." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//sends a sleep packet to the robot
void sendSleepPacket(MySocket& robotSocket, unsigned short packetCount) {

    std::cout << "--- SLEEP Packet ---" << std::endl;

    //1 create packet for sleep cmd
    PktDef sleepCommand;
    sleepCommand.setCommand(PktDef::SLEEP);
    sleepCommand.SetPktCount(packetCount);

    //2 set sleep body (empty)
    sleepCommand.SetBodyData(nullptr, 0);

    //3 calc crc
    sleepCommand.CalcCRC();
    std::cout << "Sleep CRC: " << static_cast<int>(sleepCommand.getCRC()) << std::endl;

    //4 generate raw packet data
    char* rawDataSleep = sleepCommand.GenPacket();
    char dataLengthSleep = sleepCommand.GetLength();

    //debug prints
    sleepCommand.printPacketDetails();
    sleepCommand.printBuffer(rawDataSleep, dataLengthSleep);
    std::cout << "Sending Data Length: " << static_cast<int>(dataLengthSleep) << std::endl;

    //5 send to robot
    std::cout << "Sending sleep packet..." << std::endl;
    robotSocket.SendData(rawDataSleep, dataLengthSleep);
    std::cout << "Sent sleep packet." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
}


//sends a telemetry request packet to the robot
void sendTelemetryRequestPacket(MySocket& robotSocket, unsigned short packetCount) {

    std::cout << "---TELEMETRY Packet ---" << std::endl;

    //1 make packet for telemetry request cmd
    PktDef resCommand;
    resCommand.setCommand(PktDef::RESPONSE);
    resCommand.SetPktCount(packetCount);

    //2 set body data (empty)
    resCommand.SetBodyData(nullptr, 0);

    //3 calc crc
    resCommand.CalcCRC();

    //4 generate raw packet data
    char* requestData = resCommand.GenPacket();
    char requestLength = resCommand.GetLength();

    //5 send command to robot
    std::cout << "Sending telemetry request packet." << std::endl;
    robotSocket.SendData(requestData, requestLength);
    std::cout << "Sent telemetry request packet." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

}


//recieveing a telemetry response from the robot
//robot sends 2 packets - first an ack the second with response data
std::string receiveTelemetryResponse(MySocket& robotSocket) {
    // --- ACK packet ---
    std::cout << "--- waiting for ACK from robot ---" << std::endl;
    char ackBuffer[DEFAULT_SIZE];
    memset(ackBuffer, 0, DEFAULT_SIZE);
    int ackBytesReceived = 0;
    try {
        ackBytesReceived = robotSocket.GetData(ackBuffer);
        std::cout << "Received ACK packet: " << ackBytesReceived << " bytes" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "error receiving ACK: " << e.what() << std::endl;
        return "fail receiving ACK";
    }

    if (ackBytesReceived <= 0) {
        std::cerr << "no ACK received from robot" << std::endl;
        return "fail, no ACK received";
    }

    //create the ack packet and print details to console
    PktDef ackPacket(ackBuffer, ackBytesReceived);
    std::cout << "--- ACK Packet Details ---" << std::endl;
    ackPacket.printPacketDetails();
    ackPacket.printBuffer(ackBuffer, ackBytesReceived);

    //cehck if ack crc is valid
    bool ackValidCRC = ackPacket.CheckCRC(ackBuffer, ackBytesReceived);
    if (!ackValidCRC) {
        std::cerr << "ACK CRC check failed" << std::endl;
        return "ACK CRC check failed";
    }
    std::cout << "ACK CRC check passed" << std::endl;

    //now get ready to receive telemetry data
    std::cout << "\n--- Waiting for telemetry data from robot ---" << std::endl;
    char telemetryBuffer[DEFAULT_SIZE];
    memset(telemetryBuffer, 0, DEFAULT_SIZE);
    int telemetryBytesReceived = 0;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try {
        telemetryBytesReceived = robotSocket.GetData(telemetryBuffer);
        if (telemetryBytesReceived <= 0) {
            std::cerr << "No telemetry data received from robot" << std::endl;
            return "fail: No telemetry data received";
        }
        std::cout << "Received telemetry packet: " << telemetryBytesReceived << " bytes" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error receiving telemetry: " << e.what() << std::endl;
        return "fail: Error receiving telemetry";
    }

    //packet size check, to debug getting the correct packet
    if (telemetryBytesReceived < (sizeof(PktDef::Header) + sizeof(unsigned char))) {
        std::cerr << "Error: Received packet too small to be valid telemetry" << std::endl;
        return "fail: Received telemetry packet too small";

        //create the telemetry packet and print details
        PktDef telemetryPacket(telemetryBuffer, telemetryBytesReceived);
        std::cout << "--- Telemetry Packet Details ---" << std::endl;
        telemetryPacket.printPacketDetails();
        telemetryPacket.printBuffer(telemetryBuffer, telemetryBytesReceived);

        //check Header Length vs Received Bytes
        int headerLength = telemetryPacket.GetLength();

        //check CRC
        bool telemetryValidCRC = telemetryPacket.CheckCRC(telemetryBuffer, telemetryBytesReceived);
        if (!telemetryValidCRC) {
            std::cerr << "Telemetry CRC check failed" << std::endl;
            return "fail: Telemetry CRC check failed";
        }
        std::cout << "Telemetry CRC check passed" << std::endl;

        //get pointer for the body data
        char* telemetryDataPtr = telemetryPacket.GetBodyData();
        if (telemetryDataPtr == nullptr) {
            std::cerr << "Error: No telemetry data pointer from packet body" << std::endl;
            return "fail: No telemetry body pointer";
        }

        //telemetry struct to hold the info
#pragma pack(push, 1)
        struct TelemetryData {
            unsigned short int LastPktCounter;
            unsigned short int CurrentGrade;
            unsigned short int HitCount;
            unsigned char LastCmd;
            unsigned char LastCmdValue;
            unsigned char LastCmdSpeed;
        };
#pragma pack(pop)
        const int expectedBodySize = sizeof(TelemetryData);

        //actual body size (based on header length)
        int actualBodySize = headerLength - sizeof(PktDef::Header) - sizeof(unsigned char);

        std::cout << "\n--- Telemetry Information ---" << std::endl;
        std::cout << "Actual Body size: " << actualBodySize << " bytes (Expected: " << expectedBodySize << " bytes)" << std::endl;

        //check body size is right then copy data
        if (actualBodySize >= expectedBodySize) {
            TelemetryData localTelemetry;
            memcpy(&localTelemetry, telemetryDataPtr, expectedBodySize);

            //format the string to return
            std::stringstream telemetryResult;
            telemetryResult << "Last Packet Counter: " << localTelemetry.LastPktCounter << "\n"
                << "Current Grade: " << localTelemetry.CurrentGrade << "\n"
                << "Hit Count: " << localTelemetry.HitCount << "\n"
                << "Last Command: ";
            switch (localTelemetry.LastCmd) {
            case PktDef::FORWARD:   telemetryResult << "FORWARD"; break;
            case PktDef::BACKWARDS: telemetryResult << "BACKWARDS"; break;
            case PktDef::RIGHT:     telemetryResult << "RIGHT"; break;
            case PktDef::LEFT:      telemetryResult << "LEFT"; break;
            default: telemetryResult << "UNKNOWN (" << static_cast<int>(localTelemetry.LastCmd) << ")"; break;
            }
            telemetryResult << "\n"
                << "Last Command Duration: " << static_cast<int>(localTelemetry.LastCmdValue) << " (seconds)\n"
                << "Last Command Speed: " << static_cast<int>(localTelemetry.LastCmdSpeed) << " (percent of max)";

            return telemetryResult.str();

        }
        else { //actualBodySize < expectedBodySize
            return "telemetry fail, body data too small";
        }
    }
}


void receiveAndDiscardAck(MySocket& socket, const std::string& commandName) {
    std::cout << "--- Waiting for ACK after " << commandName << " ---" << std::endl;
    char discardBuffer[DEFAULT_SIZE]; 
    int bytesReceived = 0;
    try {
        bytesReceived = socket.GetData(discardBuffer);
        if (bytesReceived > 0) {
            std::cout << "Received and discarded " << bytesReceived << " bytes - ACK for " << commandName << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Warning: Error trying to receive ACK for " << commandName << ": " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Warning: Unknown error trying to receive ACK for " << commandName << "." << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


