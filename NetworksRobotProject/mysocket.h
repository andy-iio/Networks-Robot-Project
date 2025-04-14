//milestone 2 - andyg 
#pragma once

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdexcept>

#pragma comment(lib, "Ws2_32.lib")

enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;
    SOCKET ConnectionSocket;
    sockaddr_in SvrAddr;
    SocketType mySocket;
    std::string IPAddr;
    int Port;
    ConnectionType connectionType;
    int MaxSize;

public:
    MySocket(SocketType type, std::string ipAddress, unsigned int port,
        ConnectionType connType = UDP, unsigned int bufferSize = DEFAULT_SIZE);
    ~MySocket();

    void SendData(const char* data, int size);
    int GetData(char* destination);

    std::string GetIPAddr();
    void SetIPAddr(std::string ipAddress);
    int GetPort();
    void SetPort(int port);
    SocketType GetType();
    void SetType(SocketType type);

private:
    void Cleanup();
};