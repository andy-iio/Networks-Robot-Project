//milestone 2
#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <cstring>

enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;//to dynamically allocate RAW buffer space for communication activities
    int WelcomeSocket;//used by a MySocket object configured as a TCP/IP Server
    int ConnectionSocket;//used for client/server communications (both TCP and UDP)
    struct sockaddr_in SvrAddr;//to store connection information
    SocketType mySocket;//t to hold the type of socket the MySocket object is initialized to
    std::string IPAddr;//to hold the IPv4 IP Address string
    int Port;//to hold the port number to be used
    ConnectionType connectionType;//to define the Transport Layer protocol being used (TCP/UDP)
    bool bTCPConnect; //flag to determine if a connection has been established or not
    int MaxSize; //store the maximum number of bytes the buffer is allocated to

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
    void ConnectTCP();
    void DisconnectTCP();

private:
    void Cleanup();
};
