#ifndef ROBOTCOMMANDS_H
#define ROBOTCOMMANDS_H

#include <string>
#include <memory>     
#include "mysocket.h"

void sendDrivePacket(MySocket& robotSocket, unsigned short packetCount, unsigned char direction, unsigned char duration, unsigned char speed);
void sendSleepPacket(MySocket& robotSocket, unsigned short packetCount);
void sendTelemetryRequestPacket(MySocket& robotSocket, unsigned short packetCount);
std::string  receiveTelemetryResponse(MySocket& robotSocket);
void receiveAndDiscardAck(MySocket& socket, const std::string& commandName);


#endif