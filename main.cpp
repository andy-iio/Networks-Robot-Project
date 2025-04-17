// milestone 3
#define CROW_MAIN
#define CROW_ENABLE_LOGGING // for debugging
#include "crow_all.h"
#include "mysocket.h"
#include "robotCommands.h"
#include "packet.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string> 
#include <limits> 

using namespace std;

//shared pointer to the socket, so it can be used in all function calls
//for reference:https://en.cppreference.com/w/cpp/memory/shared_ptr
std::shared_ptr<MySocket> robotSocket = nullptr;

//function to get and parse the data from json, to avoid issues
unsigned char dataFromJson(const crow::json::rvalue& node, unsigned char default_val);

//keeps track of number of packet sent and be able to update pkt count
unsigned short packetCounter = 0;

int main() {
    crow::SimpleApp app;
    int port_to_listen_on = 8080; //INSIDE the container

    // main page
    CROW_ROUTE(app, "/")
        ([](const crow::request& req, crow::response& res) {
        std::ifstream in("../public/index.html");
        if (in) {
            std::ostringstream contents;
            contents << in.rdbuf();
            in.close();
            res.set_header("Content-Type", "text/html");
            res.code = 200;
            res.write(contents.str());
        }
        else {
            res.code = 404;
            res.write("Not Found: index.html");
        }
        res.end();
            });

    // POST - connect to the robot
    CROW_ROUTE(app, "/connect/<string>/<int>").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, crow::response& res, std::string ip, int port) {
        std::cout << "Connect route called for " << ip << ":" << port << std::endl;
        try {
            // create socket with ip & port.
            robotSocket = std::make_shared<MySocket>(SocketType::CLIENT, ip, port, ConnectionType::UDP);

            std::cout << "Socket created/ready for " << ip << ":" << port << std::endl;
            res.code = 200;
            res.write("Connected successfully to " + ip + ":" + std::to_string(port));
            packetCounter = 0; //reset packet numbers after connecting again
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to create socket: " << e.what() << std::endl;
            robotSocket = nullptr;
            res.code = 500;
            res.write("Failed to connect: " + std::string(e.what()));
        }
        catch (...) {
            std::cerr << "Failed to create socket (unknown error)" << std::endl;
            robotSocket = nullptr; //needs to be null if fails
            res.code = 500;
            res.write("Failed to connect (unknown error)");
        }
        res.end();
            });

    CROW_ROUTE(app, "/telecommand/").methods(crow::HTTPMethod::PUT)
        ([](const crow::request& req, crow::response& res) {
        if (!robotSocket) {
            res.code = 400; //400 = bad request
            res.write("Robot not connected!! please connect first.");
            res.end();
            return;
        }

        crow::json::rvalue body_json;
        body_json = crow::json::load(req.body);

        //check command exists before accessing the .s()
        if (!body_json.has("command")) {
            res.code = 400;
            res.write("Missing 'command' field in JSON body.");
            res.end();
            return;
        }
        std::string command = body_json["command"].s();
        std::cout << "received command - PUT /telecommand/: " << command << std::endl;

        try {
            if (command == "drive") {
                //get the parameters the user input from json for drive
                unsigned char direction = PktDef::FORWARD; //default
                unsigned char speed = 80;       //default
                unsigned char duration = 5;      //default

                if (body_json.has("direction")) {
                    direction = dataFromJson(body_json["direction"], PktDef::FORWARD);
                }
                if (body_json.has("speed")) {
                    speed = dataFromJson(body_json["speed"], 50);
                }
                if (body_json.has("duration")) {
                    duration = dataFromJson(body_json["duration"], 5);
                }

                std::cout << "recieved DRIVE params: direction=" << static_cast<int>(direction)
                    << ", speed=" << static_cast<int>(speed)
                    << ", duration=" << static_cast<int>(duration) << std::endl;

                //increase pkt count and send the packet
                packetCounter++;
                sendDrivePacket(*robotSocket, packetCounter, direction, duration, speed);

                //receive and discard the ACK for drive
                receiveAndDiscardAck(*robotSocket, "Drive");

                res.code = 200;
                res.write("Drive command sent successfully.");

            }
            else if (command == "sleep") {
                //increase pkt count and send the packet
                packetCounter++;
                sendSleepPacket(*robotSocket, packetCounter);

                //receive and discard the ACK for sleep
                receiveAndDiscardAck(*robotSocket, "Sleep");

                res.code = 200;
                res.write("Sleep command sent successfully.");
            }
            else {
                res.code = 400;
                res.write("Unknown command: " + command);
            }
        }
        //for specific runtime errors
        catch (const std::runtime_error& e) {
            std::cerr << "Runtime error executing command '" << command << "': " << e.what() << std::endl;
            res.code = 500; //500 = server error
            res.write("Runtime error executing command '" + command + "': " + e.what());
        }
        catch (...) { //for every other error
            std::cerr << "Unknown error executing command '" << command << "'." << std::endl;
            res.code = 500; //500 = server error
            res.write("Unknown error executing command '" + command + "'.");
        }
        res.end();
            });


    //GET route for requesting telemetry
    CROW_ROUTE(app, "/telementry_request/").methods(crow::HTTPMethod::GET)
        ([](crow::response& res) {
        //check if the connection was established
        if (!robotSocket) {
            res.write("Robot not connected!! please connect first.");
            res.end();
            return;
        }

        std::cout << "Telemetry request received. Sending request packet..." << std::endl;
        try {
            packetCounter++; //increase count
            sendTelemetryRequestPacket(*robotSocket, packetCounter);
            std::string telemetry_data = receiveTelemetryResponse(*robotSocket);

            std::cout << "Received telemetry data string:\n" << telemetry_data << std::endl;
            res.code = 200; //200 = ok
            res.set_header("Content-Type", "text/plain"); //send back as plain text
            res.write(telemetry_data); //write to page
        }
        //for specific errors
            catch (const std::exception& e) {
            std::cerr << "Error during telemetry operation: " << e.what() << std::endl;
            CROW_LOG_ERROR << "Error during telemetry operation: " << e.what();
            res.code = 500;
            res.write("Error during telemetry operation: " + std::string(e.what()));
        }
        //for every other error
        catch (...) {
            std::cerr << "Unknown error during telemetry operation." << std::endl;
            res.code = 500;
            res.write("Unknown error during telemetry operation.");
        }
        res.end();
            });


    //styles routes for css files
    CROW_ROUTE(app, "/styles/<path>")
        ([](const crow::request& req, crow::response& res, const string& path) {
        std::string file_path = "../public/styles/" + path;
        std::ifstream in(file_path, std::ios::in | std::ios::binary);
        if (in) {
            std::ostringstream contents; contents << in.rdbuf(); in.close();
            res.set_header("Content-Type", "text/css");
            res.code = 200; res.write(contents.str());
        }
        else {
            res.code = 404; res.write("Not Found: CSS file");
            std::cerr << "ERROR: Failed to open " << file_path << std::endl;
        }
        res.end();
            });

    //scripts route for js files
    CROW_ROUTE(app, "/scripts/script.js")
        ([](const crow::request& req, crow::response& res) {
        std::string filePath = "../public/scripts/script.js";
        std::ifstream in(filePath, std::ios::in | std::ios::binary);
        if (in) {
            std::ostringstream contents; contents << in.rdbuf(); in.close();
            res.set_header("Content-Type", "application/javascript");
            res.code = 200; res.write(contents.str());
            std::cout << "Served: /scripts/script.js -> " << filePath << std::endl;
        }
        else {
            res.code = 404; res.write("Not Found: script.js");
            std::cerr << "ERROR: Failed to open " << filePath << " for /scripts/script.js" << std::endl;
        }
        res.end();
            });

    app.port(port_to_listen_on).multithreaded().run();

    return 0;
}

//function to safely get data from crow::json::rvalue
unsigned char dataFromJson(const crow::json::rvalue& node, unsigned char default_val) {

    try {
        //crow numbers stored as double or int
        double val_d = node.d(); //get as double first
        if (val_d >= 0 && val_d <= std::numeric_limits<unsigned char>::max() && val_d == floor(val_d)) {
            return static_cast<unsigned char>(val_d);
        }
        //get int if cant get double
        long long val_i = node.i();
        if (val_i >= 0 && val_i <= std::numeric_limits<unsigned char>::max()) {
            return static_cast<unsigned char>(val_i);
        }
        std::cerr << "json value error" << node << std::endl;
        return default_val; //default value
    }
    catch (const std::exception& e) {
        std::cerr << "error getting JSON value: " << e.what() << std::endl;
        return default_val;
    }
}