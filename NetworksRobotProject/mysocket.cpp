//milestone 2 - andyg 
#include "mysocket.h"

MySocket::MySocket(SocketType type, std::string ipAddress, unsigned int port,
    ConnectionType connType, unsigned int bufferSize) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    mySocket = type;
    IPAddr = ipAddress;
    Port = port;
    connectionType = connType;

    MaxSize = (bufferSize > 0) ? bufferSize : DEFAULT_SIZE;

    Buffer = new char[MaxSize];
    memset(Buffer, 0, MaxSize);

    ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Always UDP
    if (ConnectionSocket == INVALID_SOCKET) {
        Cleanup();
        throw std::runtime_error("couldnt create socket");
    }

    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);

    if (mySocket == SERVER) {
        SvrAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            Cleanup();
            throw std::runtime_error("couldnt bind");
        }
    }
    else if (mySocket == CLIENT) {
        inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);
    }
}

MySocket::~MySocket() {
    Cleanup();
}

void MySocket::SendData(const char* data, int size) {
    if (size <= 0 || size > MaxSize) {
        throw std::runtime_error("Invalid data size");
    }

    int bytesSent = sendto(ConnectionSocket, data, size, 0,
        (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
    if (bytesSent == SOCKET_ERROR) {
        throw std::runtime_error("error sending: " + std::to_string(WSAGetLastError()));
    }
}

int MySocket::GetData(char* destination) {
    if (destination == nullptr) {
        throw std::runtime_error("Invalid destination buffer");
    }

    sockaddr_in senderAddr;
    int addrLen = sizeof(senderAddr);

    int bytesReceived = recvfrom(ConnectionSocket, Buffer, MaxSize, 0,
        (struct sockaddr*)&senderAddr, &addrLen);
    if (bytesReceived == SOCKET_ERROR) {
        throw std::runtime_error("error recieving: " + std::to_string(WSAGetLastError()));
    }

    memcpy(destination, Buffer, bytesReceived);
    return bytesReceived;
}

std::string MySocket::GetIPAddr() {
    return IPAddr;
}

void MySocket::SetIPAddr(std::string ipAddress) {
    IPAddr = ipAddress;
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);
}

int MySocket::GetPort() {
    return Port;
}

void MySocket::SetPort(int port) {
    Port = port;
    SvrAddr.sin_port = htons(Port);
}

SocketType MySocket::GetType() {
    return mySocket;
}

void MySocket::SetType(SocketType type) {
    mySocket = type;
}

void MySocket::Cleanup() {
    if (ConnectionSocket != INVALID_SOCKET) {
        closesocket(ConnectionSocket);
        ConnectionSocket = INVALID_SOCKET;
    }

    if (Buffer != nullptr) {
        delete[] Buffer;
        Buffer = nullptr;
    }

    WSACleanup();
}