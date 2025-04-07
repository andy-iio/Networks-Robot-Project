//this file exists to be able to run the tests, and debug
#include "packet.h"
#include <iostream>
#include <iomanip>

int main() {
    //----packet 1----//
    //creating a drive command packet, print details
    std::cout << "--drive packet--" << std::endl;
    PktDef drivePkt;
    drivePkt.setCommand(PktDef::DRIVE);
    drivePkt.SetPktCount(1);

    //settig drive data
    struct DriveData {
        unsigned char direction;
        unsigned char duration;
        unsigned char speed;
    } driveData = { PktDef::FORWARD, 10, 100 };  //10 duration, speed 100

    drivePkt.SetBodyData(reinterpret_cast<char*>(&driveData), sizeof(DriveData));
    drivePkt.CalcCRC();
    drivePkt.printPacketDetails();

    //generate packet, print details
    char* rawDriveData = drivePkt.GenPacket();
    int driveLength = drivePkt.GetLength();
    drivePkt.printBuffer(rawDriveData, driveLength);

    std::cout << "CRC check: " << (drivePkt.CheckCRC(rawDriveData, driveLength) ? "PASSED" : "FAILED") << std::endl;
    std::cout << std::endl;

    //----packet 2----//
    //createing a sleep packet, print details
    std::cout << "--sleep packet--" << std::endl;
    PktDef sleepPkt;
    sleepPkt.setCommand(PktDef::SLEEP);
    sleepPkt.SetPktCount(2);
    sleepPkt.CalcCRC();
    sleepPkt.printPacketDetails();

    //generate packet with genpacket(), print details
    char* rawSleepData = sleepPkt.GenPacket();
    int sleepLength = sleepPkt.GetLength();
    sleepPkt.printBuffer(rawSleepData, sleepLength);

    std::cout << "CRC check: " << (sleepPkt.CheckCRC(rawSleepData, sleepLength) ? "PASSED" : "FAILED") << std::endl;
    std::cout << std::endl;


    //----packet 3----//
    //recieving a packet
    std::cout << "--receiving a packet--" << std::endl;

    //pretending the drive packet is the recieved data now
    PktDef receivedPkt(rawDriveData, driveLength);

    std::cout << "packet details:" << std::endl;
    receivedPkt.printPacketDetails();

    //print body data if if it's a drive command
    if (receivedPkt.GetCmd() == PktDef::DRIVE && receivedPkt.GetBodyData() != nullptr) {
        DriveData* recvDriveData = reinterpret_cast<DriveData*>(receivedPkt.GetBodyData());
        std::cout << "direction: " << static_cast<int>(recvDriveData->direction) << std::endl;
        std::cout << "duration: " << static_cast<int>(recvDriveData->duration) << std::endl;
        std::cout << "speed: " << static_cast<int>(recvDriveData->speed) << std::endl;
    }
    std::cout << std::endl;


    //----packet 4----//
    //createing a response packet, print details
    std::cout << "--response packet--" << std::endl;
    PktDef responsePkt;
    responsePkt.setCommand(PktDef::RESPONSE);
    responsePkt.SetPktCount(2);
    responsePkt.CalcCRC();
    responsePkt.printPacketDetails();

    //generate packet with genpacket(), print details
    char* rawResponseData = responsePkt.GenPacket();
    int responseLength = responsePkt.GetLength();
    responsePkt.printBuffer(rawResponseData, responseLength);

    std::cout << "CRC check: " << (responsePkt.CheckCRC(rawResponseData, responseLength) ? "PASSED" : "FAILED") << std::endl;
    std::cout << std::endl;


    return 0;
}

