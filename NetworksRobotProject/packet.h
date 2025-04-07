#pragma once
#include <memory>
#include <iostream>
#include <fstream>
#define MAX_PACKET_SIZE 1024

const int EmptyPktSize = 6;					//Number of data bytes in a packet with no data field

class PktDef
{
    //A structure Header which contains the header information based on the description above
    struct Header
    {
        unsigned short Length;					    //Number of characters in the line
        unsigned short PktCount;
        unsigned char Drive : 1;
        unsigned char Status : 1;
        unsigned char Sleep : 1;
        unsigned char Ack : 1;
        unsigned char Padding : 4;
    };

    //A structure DriveBody which contains the drive parameter information based on the description above
    struct DriveBody {
        unsigned char Direction;
        unsigned char Duration;
        unsigned char Speed;
    };

    //An enumerated CmdType to define the command types {DRIVE, SLEEP, RESPONSE}
public: enum CmdType {
        DRIVE,
        SLEEP,
        RESPONSE
        };

    //The following constant integer definitions, matching the values previously presented
    static const int FORWARD = 1;
    static const int BACKWARDS = 2;
    static const int RIGHT = 3;
    static const int LEFT = 4;
    static const int HEADERSIZE = 5;

    //A private structure to define a CmdPacket
private:
    struct CmdPacket {
        Header header;
        char* Data;							//The data bytes
        unsigned int CRC;					//Cyclic Redundancy Chec
    } cmdpacket;

    //A char *RawBuffer that will store all data in PktDef in a serialized form that can be used to transmit to the robot
    char* RawBuffer;

public:
    //A default constructor that places the PktDef object in a safe state
    //andy
    PktDef() : RawBuffer(nullptr){ //initializing head with default values
        memset(&cmdpacket.header, 0, sizeof(Header));
        cmdpacket.Data = nullptr;
        cmdpacket.CRC = 0;
    };

    // An overloaded constructor that takes a RAW data buffer, parses the data and populates the Header, Body, and CRC contents of the PktDef objec
    //alexa
    PktDef(char* Buffer, int length) { // takes in buffer, length of buffer, and init the buffer

        RawBuffer = nullptr;
        if (Buffer == nullptr || length <= 0) {
            //if so, create empty packet, input is bad
            memset(&cmdpacket.header, 0, sizeof(Header));
            cmdpacket.Data = nullptr;
            cmdpacket.CRC = 0;
            return;
        }

        // copying buffer 
        RawBuffer = new char[length];
        memcpy(RawBuffer, Buffer, length);

        // header
        memcpy(&cmdpacket.header, Buffer, sizeof(Header));

        //body                                  
        int body = length - sizeof(Header) - sizeof(unsigned int);
        if (body > 0) { // nullptr check for body 
            cmdpacket.Data = new char[body];
            //coping the body data 
            memcpy(cmdpacket.Data, RawBuffer + sizeof(Header), body);
        }
        else {
            cmdpacket.Data = nullptr;
        }

        //CRC
        memcpy(&cmdpacket.CRC, RawBuffer + length - sizeof(unsigned int), sizeof(unsigned int));
    }

    //destructor to clean the memory after
    //andy
    ~PktDef() {
        if (cmdpacket.Data != nullptr) {
            delete[] cmdpacket.Data;
        }
        if (RawBuffer != nullptr) {
            delete[] RawBuffer;
        }
    }

    //A set function that sets the packets command flag based on the CmdType
    void setCommand(CmdType command) {
        //reset everything to 0 first
        cmdpacket.header.Drive = 0;
        cmdpacket.header.Sleep = 0;
        cmdpacket.header.Status = 0;
        cmdpacket.header.Ack = 0;
        
        switch (command) {
        case DRIVE:
            cmdpacket.header.Drive = 1;
            break;
        case SLEEP:
            cmdpacket.header.Sleep = 1;
            break;
        case RESPONSE:
            cmdpacket.header.Status = 1;
            cmdpacket.header.Ack = 1;
            break;

        default:
            cmdpacket.header.Sleep = 1; //set default to sleep for safety
            break;
        }
    }

    //a set function that takes a pointer to a RAW data buffer and the size of the buffer in bytes. 
    //This function will allocate the packets Body field and copies the provided data into the objects buffer
    //andy
    void SetBodyData(char* data, int size) {
        if (cmdpacket.Data != nullptr) {
            delete[] cmdpacket.Data;
            cmdpacket.Data = nullptr;
        }
        //sets length to the new length
        cmdpacket.header.Length = size;

        //allocates the packets body field and copies the data
        if (size > 0 && data != nullptr) {
            cmdpacket.Data = new char[size];
            memcpy(cmdpacket.Data, data, size);
        }
    }

    // a set function that sets the objects PktCount header variable
    //alexa
    void SetPktCount(int count) {
        cmdpacket.header.PktCount = count; //setting the Pktcount to the int that is passed in
    }

    //a query function that returns the CmdType based on the set command flag bit
    //andy
    CmdType GetCmd() {
        if (cmdpacket.header.Drive) {
            return DRIVE;
        }
        else if (cmdpacket.header.Sleep) {
            return SLEEP;
        }
        else if (cmdpacket.header.Status) {
            return RESPONSE;
        }
        else {
            return SLEEP; //setting sleep as default if no command can be found in header
        }
    }

    // a query function that returns True/False based on the Ack flag in the header
    //andy
    bool GetAck() {
        return cmdpacket.header.Ack == 1;
    }

    //a query function that returns the packet Length in bytes
    //alexa
    int GetLength() {
        int bodySize = (cmdpacket.Data != nullptr) ? cmdpacket.header.Length : 0;
        return sizeof(Header) + bodySize + sizeof(unsigned int);
        //return sizeof(cmdpacket.header) + sizeof(cmdpacket.Data) + sizeof(cmdpacket.CRC); //adding up all parts to return the full packet length 
    }

    //a query function that returns a pointer to the objects Body field
    //andy
    char* GetBodyData() {
        return cmdpacket.Data;
    }

    //a query function that returns the PktCount value
    //alexa
    int GetPktCount() {
        return cmdpacket.header.PktCount; // return PKcount 
    }

    // a function that takes a pointer to a RAW data buffer, the
    //size of the buffer in bytes, and calculates the CRC. If the calculated CRC matches the
    //CRC of the packet in the buffer the function returns TRUE, otherwise FALSE.
    //andy
    bool CheckCRC(char* RawBuffer, int size) {
        if (RawBuffer == nullptr || size <= sizeof(Header)) {
            return false; //if an invalid buffer or if too small
        }

        //find size of data (not including crc, which is an unsigned int)
        int dataSize = size - sizeof(unsigned int);

        //count all the 1's not including the crc
        unsigned char calculatedCrc = 0;
        for (int i = 0; i < dataSize; i++) {
            unsigned char byte = RawBuffer[i];
            while (byte) { //loop for each bit of the byte (8)
                calculatedCrc += byte & 1; //increase calculatedcrc by 1 for each 1 in the byte, by checking the lsb
                byte >>= 1; //shift byte one bit (to the right)
            }
        }

        //get crc from end of buffer
        unsigned char receivedCrc;
        memcpy(&receivedCrc, RawBuffer + dataSize, sizeof(unsigned char));

        //check if the calculated crc is the same as the crc we received
        return receivedCrc == calculatedCrc;
    }

    //a function that calculates the CRC and sets the objects packet CRC parameter
    //andy
    void CalcCRC() {
        int bodySize = (cmdpacket.Data != nullptr) ? cmdpacket.header.Length : 0;
        int totalSize = sizeof(Header) + bodySize;
        char* buffer = new char[totalSize];

        //copy over the header and body data(if there is any)
        memcpy(buffer, &cmdpacket.header, sizeof(Header));
        if (bodySize > 0 && cmdpacket.Data != nullptr) {
            memcpy(buffer + sizeof(Header), cmdpacket.Data, bodySize);
        }

        unsigned char crc = 0;
        //count all 1's, not including the crc 
        for (int i = 0; i < totalSize; i++) {
            unsigned char byte = buffer[i];
            while (byte) { //loop for each bit of the byte (8)
                crc += byte & 1; //increase crc by 1 for each 1 in the byte, by checking the lsb
                byte >>= 1; //shift byte one bit (to the right)
            }
        }
        cmdpacket.CRC = crc; //set crc
        delete[] buffer; //clean up
    }

    // a function that allocates the private RawBuffer and transfers the
    //contents from the objects member variables into a RAW data packet (RawBuffer) for
    //transmission. The address of the allocated RawBuffer is returned
    //andy
    char* GenPacket() {
        //clear old one
        if (RawBuffer != nullptr) {
            delete[] RawBuffer;
            RawBuffer = nullptr;
        }

        //sanity check - make sure length is not negative
        int bodySize = (cmdpacket.Data != nullptr) ? cmdpacket.header.Length : 0;

        //finding the total size of the packet, and allocating buffer with the new size
        int totalSize = sizeof(Header) + bodySize + sizeof(unsigned char); //useing unsigned char for CRC
        RawBuffer = new char[totalSize];

        //copy over the header
        memcpy(RawBuffer, &cmdpacket.header, sizeof(Header));

        //copy data if it exists
        if (bodySize > 0 && cmdpacket.Data != nullptr) {
            memcpy(RawBuffer + sizeof(Header), cmdpacket.Data, bodySize);
        }

        //copy crc (as a byte)
        unsigned char crc = static_cast<unsigned char>(cmdpacket.CRC);
        memcpy(RawBuffer + sizeof(Header) + bodySize, &crc, sizeof(unsigned char));

        return RawBuffer;
    }

    //to help with debugging
    //andy
    void printPacketDetails() {
        std::cout << "command type: ";
        switch (GetCmd()) {
        case PktDef::DRIVE: std::cout << "DRIVE"; break;
        case PktDef::SLEEP: std::cout << "SLEEP"; break;
        case PktDef::RESPONSE: std::cout << "RESPONSE"; break;
        default: std::cout << "???"; break;
        }
        std::cout << std::endl;
        std::cout << "packet count: " << GetPktCount() << std::endl;
        std::cout << "packet length: " << GetLength() << " bytes" << std::endl;
        std::cout << "ack flag: " << (GetAck() ? "Yes" : "No") << std::endl;

        char* body = GetBodyData();
        if (body != nullptr) {
            std::cout << "has body data" << std::endl;
        }
        else {
            std::cout << "no body data" << std::endl;
        }
    }

    //print buffer data in hex
    //andy
    void printBuffer(const char* buffer, int size) {
        std::cout << "buffer data [" << size << " bytes]: ";
        for (int i = 0; i < size; i++) {
            std::cout << std::hex << static_cast<int>(static_cast<unsigned char>(buffer[i])) << " ";
        }
        std::cout << std::dec << std::endl;
    }
};