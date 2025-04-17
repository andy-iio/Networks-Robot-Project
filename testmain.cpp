//this file is to test the packet/socket with the simulator 
//before the gui gets setup

#include "mysocket.h"
#include "packet.h"
#include <iostream>
#include <chrono>
#include <thread>

int testmain() {
    //UDP client socket
    MySocket robotSocket(CLIENT, "10.0.0.129", 5009, TCP);

    try {
        robotSocket.ConnectTCP();
        std::cout << "TCP connection established successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to establish TCP connection: " << e.what() << std::endl;
        return 1;
    }

    //----------------------------------
    //DRIVE PACKET TEST
    std::cout << "--- DRIVE PACKET ---" << std::endl;

    //1 set up packet
    PktDef driveCommand;
    driveCommand.setCommand(PktDef::DRIVE);
    driveCommand.SetPktCount(1);

    //set drive parameters
    struct {
        unsigned char direction;
        unsigned char duration;
        unsigned char speed;
    } driveData = { PktDef::LEFT, 90, 80 };

    //2 set body
    driveCommand.SetBodyData(reinterpret_cast<char*>(&driveData), sizeof(driveData));

    //debugging
    std::cout << "Sizeof driveData: " << sizeof(driveData) << std::endl;
    std::cout << "cmdpacket.header.Length: " << (int)driveCommand.GetLength() << std::endl;
    std::cout << "cmdpacket.getack: " << (int)driveCommand.GetAck() << std::endl;

    //3 calculate crc
    driveCommand.CalcCRC();
    std::cout << "drive crc " << (int)driveCommand.getCRC() << std::endl;

    //4 generate the raw packet data
    char* rawData = driveCommand.GenPacket();
    char dataLength = driveCommand.GetLength();

    //debug prints
    std::cout << "--- Sending DRIVE Packet ---" << std::endl;
    driveCommand.printPacketDetails();
    driveCommand.printBuffer(rawData, dataLength);
    std::cout << "Data Length: " << dataLength << std::endl;  // VERY IMPORTANT

    //5 send the command to the robot
    std::cout << "sending drive packet" << std::endl;
    robotSocket.SendData(rawData, dataLength);
    std::cout << "sent drive packet" << std::endl;


    //importantt needed to stop sending random other info accidently
    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    //----------------------------------
    //RESPONSE PACKET TEST
    std::cout << "--- RESPONSE PACKET ---" << std::endl;

    //1 make packet for response cmd
    PktDef resCommand;
    resCommand.setCommand(PktDef::RESPONSE);
    resCommand.SetPktCount(1);

    //2 set body data
    resCommand.SetBodyData(nullptr, 0);

    //3 calc crc
    resCommand.CalcCRC();

    //4 generate raw packet data
    char* responseData = resCommand.GenPacket();
    char responseLength = resCommand.GetLength();

    //5 send command to robot
    std::cout << "sending response packet" << std::endl;
    robotSocket.SendData(responseData, responseLength);
    std::cout << "sendt response packet" << std::endl;

    //importantt needed to stop sending random other info accidently
    std::this_thread::sleep_for(std::chrono::milliseconds(500));




   // //----------------------------------
   ////SLEEP PACKET TEST
   // std::cout << "--- SLEEP PACKET ---" << std::endl;

   // //1 create packet for sleep cmd
   // PktDef sleepCommand;
   // sleepCommand.setCommand(PktDef::SLEEP);
   // sleepCommand.SetPktCount(1);

   // //2 set sleep body
   // sleepCommand.SetBodyData(nullptr, 0);

   // //debugging
   // std::cout << "sleeppacket total length: " << (int)sleepCommand.GetLength() << std::endl;
   // std::cout << "sleeppacket.getack: " << (int)sleepCommand.GetAck() << std::endl;

   // //3 calc crc
   // sleepCommand.CalcCRC();
   // std::cout << "slepe crc " << (int)sleepCommand.getCRC() << std::endl;

   // //4 generate raw packet data
   // char* rawDataSleep = sleepCommand.GenPacket();
   // char dataLengthSleep = sleepCommand.GetLength();

   // //5 send to robot
   // robotSocket.SendData(rawDataSleep, dataLengthSleep);



    // First packet - ACK response
    std::cout << "--- Waiting for ACK from robot ---" << std::endl;
    char ackBuffer[DEFAULT_SIZE];
    memset(ackBuffer, 0, DEFAULT_SIZE);

    int ackBytesReceived = 0;
    try {
        ackBytesReceived = robotSocket.GetData(ackBuffer);
        std::cout << "Received ACK packet: " << ackBytesReceived << " bytes" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error receiving ACK: " << e.what() << std::endl;
        return 1;
    }

    if (ackBytesReceived <= 0) {
        std::cerr << "No ACK received from robot" << std::endl;
        return 1;
    }

    // Process the first (ACK) packet
    PktDef ackPacket(ackBuffer, ackBytesReceived);
    std::cout << "--- ACK Packet Details ---" << std::endl;
    ackPacket.printPacketDetails();
    ackPacket.printBuffer(ackBuffer, ackBytesReceived);

    // Verify ACK CRC
    bool ackValidCRC = ackPacket.CheckCRC(ackBuffer, ackBytesReceived);
    if (ackValidCRC) {
        std::cout << "ACK CRC check passed" << std::endl;
    }
    else {
        std::cout << "ACK CRC check failed" << std::endl;
        return 1;
    }

    // Small delay to make sure second packet is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Second packet - Telemetry data
    std::cout << "\n--- Waiting for telemetry data from robot ---" << std::endl;
    char telemetryBuffer[DEFAULT_SIZE];
    memset(telemetryBuffer, 0, DEFAULT_SIZE);

    int telemetryBytesReceived = 0;
    try {
        telemetryBytesReceived = robotSocket.GetData(telemetryBuffer);
        std::cout << "Received telemetry packet: " << telemetryBytesReceived << " bytes" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error receiving telemetry: " << e.what() << std::endl;
        return 1;
    }

    if (telemetryBytesReceived <= 0) {
        std::cerr << "No telemetry data received from robot" << std::endl;
        return 1;
    }

    // Process the second (telemetry) packet
    PktDef telemetryPacket(telemetryBuffer, telemetryBytesReceived);
    std::cout << "--- Telemetry Packet Details ---" << std::endl;
    telemetryPacket.printPacketDetails();
    telemetryPacket.printBuffer(telemetryBuffer, telemetryBytesReceived);

    // Verify telemetry CRC
    bool telemetryValidCRC = telemetryPacket.CheckCRC(telemetryBuffer, telemetryBytesReceived);
    if (telemetryValidCRC) {
        std::cout << "Telemetry CRC check passed" << std::endl;
    }
    else {
        std::cout << "Telemetry CRC check failed" << std::endl;
        return 1;
    }

    // Define the telemetry data structure according to the specification
#pragma pack(push, 1) // Ensure no padding between struct members
    struct TelemetryData {
        unsigned short int LastPktCounter;   // Last commanded packet counter received
        unsigned short int CurrentGrade;     // Group's current grade
        unsigned short int HitCount;         // Number of hits during demonstration
        unsigned char LastCmd;               // Last drive command received
        unsigned char LastCmdValue;          // Last drive command duration
        unsigned char LastCmdSpeed;          // Last drive command speed
    };
#pragma pack(pop)

    // Get the telemetry data from the packet body
    char* telemetryData = telemetryPacket.GetBodyData();
    if (telemetryData != nullptr) {
        // Calculate expected size of telemetry data
        const int expectedSize = sizeof(TelemetryData);

        // Get actual body size
        int bodySize = telemetryPacket.GetLength() - sizeof(PktDef::Header) - sizeof(unsigned char);

        std::cout << "\n--- Telemetry Information ---" << std::endl;
        std::cout << "Body size: " << bodySize << " bytes (Expected: " << expectedSize << " bytes)" << std::endl;

        // Check if we received enough data
        if (bodySize >= expectedSize) {
            // Cast the data to our telemetry structure
            TelemetryData* telemetry = reinterpret_cast<TelemetryData*>(telemetryData);

            // Display the telemetry information in a readable format
            std::cout << "\n=== ROBOT TELEMETRY DATA ===" << std::endl;
            std::cout << "Last Packet Counter: " << telemetry->LastPktCounter << std::endl;
            std::cout << "Current Grade: " << telemetry->CurrentGrade << std::endl;
            std::cout << "Hit Count: " << telemetry->HitCount << std::endl;

            // Convert the LastCmd to a readable direction
            std::cout << "Last Command: ";
            switch (telemetry->LastCmd) {
            case PktDef::FORWARD:
                std::cout << "FORWARD";
                break;
            case PktDef::BACKWARDS:
                std::cout << "BACKWARDS";
                break;
            case PktDef::RIGHT:
                std::cout << "RIGHT";
                break;
            case PktDef::LEFT:
                std::cout << "LEFT";
                break;
            default:
                std::cout << "UNKNOWN (" << static_cast<int>(telemetry->LastCmd) << ")";
                break;
            }
            std::cout << std::endl;

            std::cout << "Last Command Duration: " << static_cast<int>(telemetry->LastCmdValue)
                << " (seconds)" << std::endl;
            std::cout << "Last Command Speed: " << static_cast<int>(telemetry->LastCmdSpeed)
                << " (percent of max)" << std::endl;
            std::cout << "==============================" << std::endl;
        }
        else {
            std::cerr << "Warning: Received telemetry data is smaller than expected structure." << std::endl;

            // Print raw data for debugging
            std::cout << "Raw telemetry data: ";
            for (int i = 0; i < bodySize; i++) {
                std::cout << std::hex << static_cast<int>(static_cast<unsigned char>(telemetryData[i])) << " ";
            }
            std::cout << std::dec << std::endl;
        }
    }
    else {
        std::cerr << "No telemetry data in packet body" << std::endl;
    }

    try {
        robotSocket.DisconnectTCP();
        std::cout << "TCP connection closed successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error disconnecting TCP: " << e.what() << std::endl;
    }
    return 0;
}