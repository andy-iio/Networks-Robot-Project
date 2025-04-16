//milestone 2

#include "mysocket.h"

//A constructor that configures the socket and connection types, sets the IP Address and Port Number and
//dynamically allocates memory for the Buffer

MySocket::MySocket(SocketType type, std::string ipAddress, unsigned int port, ConnectionType connType, unsigned int bufferSize) {
    mySocket = type;
    IPAddr = ipAddress;
    Port = port;
    connectionType = connType;
    bTCPConnect = false;

    MaxSize = (bufferSize > 0) ? bufferSize : DEFAULT_SIZE;

    Buffer = new char[MaxSize];
    memset(Buffer, 0, MaxSize);

    if (connectionType == TCP) {
        ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    else {
        ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (ConnectionSocket < 0) {
        Cleanup();
        throw std::runtime_error("couldnt create socket");
    }

    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);

    if (mySocket == SERVER) {
        SvrAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) < 0) {
            perror("Bind failed");
            Cleanup();
            throw std::runtime_error("couldnt bind");
        }
        std::cout << "socket bound to port " << Port << std::endl;
    }
    else if (mySocket == CLIENT) {
        inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);
    }
}

//A destructor that cleans up all dynamically allocated memory space
MySocket::~MySocket() {
    Cleanup();
}

// Used to establish a TCP/IP socket connection (3-way handshake
void MySocket::ConnectTCP() {
    if (connectionType != TCP || mySocket != CLIENT) {
        throw std::runtime_error("Not TCP");
    }

    if (bTCPConnect) {
        throw std::runtime_error("TCP connection already established");
    }

    if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) < 0) {
        throw std::runtime_error("Could not connect");
    }

    bTCPConnect = true;
}

//Used to disconnect an established TCP/IP socket connection (4-way handshake
void MySocket::DisconnectTCP() {
    if (connectionType != TCP) {
        throw std::runtime_error("Not TCP");
    }

    if (!bTCPConnect) {
        throw std::runtime_error("No TCP connection established");
    }

    //close the socket
    close(ConnectionSocket);

    //create a new socket for later connections
    ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectionSocket < 0) {
        throw std::runtime_error("Couldnt create new socket after disconnecting");
    }

    bTCPConnect = false;
}

//Used to transmit a block of RAW data, specified by the starting memory address and number of bytes, over the socket.
//should work with both TCP and UDP
void MySocket::SendData(const char* data, int size) {
    if (size <= 0 || size > MaxSize) {
        throw std::runtime_error("Invalid data size");
    }

    int bytesSent;

    if (connectionType == TCP) { //send for tcp
        bytesSent = send(ConnectionSocket, data, size, 0);
    }
    else { //sendto for udp
        bytesSent = sendto(ConnectionSocket, data, size, 0,
            (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
    }

    if (bytesSent < 0) {
        std::cerr << "Error sending data. errno: " << errno << " (" << strerror(errno) << ")" << std::endl; // Log errno
        throw std::runtime_error("Error sending data");
    }
    else {
        std::cout << "Successfully sent " << bytesSent << " bytes." << std::endl; // Log success and size
    }
}

//Used to receive the last block of RAW data stored in the internal MySocket Buffer
//After getting the received message into Buffer, this function will transfer its contents to
//the provided memory address and return the total number of bytes written.This function
//should work with both TCP and UDP
int MySocket::GetData(char* destination) {
    if (destination == nullptr) {
        throw std::runtime_error("Invalid destination");
    }

    int bytesReceived;

    if (connectionType == TCP) { //recv for tcp
        bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
    }
    else { //recvfrom for udp
        sockaddr_in senderAddr;
        socklen_t addrLen = sizeof(senderAddr);
        bytesReceived = recvfrom(ConnectionSocket, Buffer, MaxSize, 0,
            (struct sockaddr*)&senderAddr, &addrLen);
    }

    if (bytesReceived < 0) {
        std::cerr << "Error receiving data. errno: " << errno << " (" << strerror(errno) << ")" << std::endl; // Log errno
        throw std::runtime_error("Error receiving data");
    }
    else if (bytesReceived == 0) {
        std::cout << "GetData received 0 bytes (graceful shutdown from peer?)." << std::endl;
    }
    else {
        std::cout << "GetData received " << bytesReceived << " bytes." << std::endl; // Log success and size
        memcpy(destination, Buffer, bytesReceived);
    }
    return bytesReceived;
}

//Returns the IP address configured within the MySocket object
std::string MySocket::GetIPAddr() {
    return IPAddr;
}

//Changes the default IP address within the MySocket object
//This method should return an error message if a connection has already been established
void MySocket::SetIPAddr(std::string ipAddress) {
    if (bTCPConnect) {
        throw std::runtime_error("connection has already been established");
    }
    IPAddr = ipAddress;
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);
}

// Returns the Port number configured within the MySocket object
int MySocket::GetPort() {
    return Port;
}

//Changes the default Port number within the MySocket object
//This method should return an error if a connection has already been established
void MySocket::SetPort(int port) {
    if (bTCPConnect) {
        throw std::runtime_error("connection has already been established");
    }
    Port = port;
    SvrAddr.sin_port = htons(Port);
}

//Returns the default SocketType the MySocket object is configured as
SocketType MySocket::GetType() {
    return mySocket;
}

//Changes the default SocketType within the MySocket object
void MySocket::SetType(SocketType type) {
    mySocket = type;
}

//cleans up and closes the socket
void MySocket::Cleanup() {
    if (ConnectionSocket >= 0) {
        close(ConnectionSocket);
        ConnectionSocket = -1;
    }

    if (Buffer != nullptr) {
        delete[] Buffer;
        Buffer = nullptr;
    }
}
