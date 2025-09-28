#include <zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include "server.h"


void RiverCore::server::reply() {
    zmq::context_t context(1);
    zmq::socket_t pull(context, zmq::socket_type::pull);
    pull.bind("tcp://*:5555");

    zmq::socket_t publish(context, zmq::socket_type::pub);
    publish.bind("tcp://*:5556");
    
    while(true) {
        zmq::message_t msg;
        auto res = pull.recv(msg, zmq::recv_flags::dontwait);
        if (res) {
            int newX, int newY;
            std::string input(static_cast<char*>(msg.data()), msg.size());
            int playerId; char cmd[16];
            sscanf(input.c_str(), "%d %s", &playerId, cmd);

            if (std::string(cmd) == "left") {
                newX = -300;
            } else if (std::string(cmd) == "right") {
                newX = 300;
            }

            std::string update = std::to_string(playerId) + " " + std::to_string(newX) + " " + std::to_string(newY);
            zmq::message_t m(update.size());
            memcpy(m.data(), update.data(), update.size());
            publish.send(m, zmq::send_flags::none);
        }
    }

    
    pull.close();
    publish.close();
    context.close();

}