#include <iostream>
#include "crow.h"


int main() {
    crow::SimpleApp app;

    // HTML Connection (retirn the main screen)
    CROW_ROUTE(app, "/")([](){
        return "<html><body><h1>GUI</h1></body></html>";
    });


    // POST
    CROW_ROUTE(app, "/connect/<string>/<int>").methods(crow::HTTPMethod::POST)([](const crow::request& req, std::string ip, int port) {

    });

    // PUT 
    CROW_ROUTE(app, "/telecommand/").methods(crow::HTTPMethod::PUT)([](const crow::request& req){

    });

    //GET
    CROW_ROUTE(app, "/telementry_request/").methods("GET"_method)([](){ 

    });

    //std::cout << "Hello from Docker build!" << std::endl;
    //return 0;
}


