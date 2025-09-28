#include <zmq.hpp>
#include <string>
#include <iostream>
#include "client.h"

static zmq::context_t context(1);
static zmq::socket_t* push = nullptr;
static zmq::socket_t* sub = nullptr;

void initNetwork() {
    push = new zmq::socket_t(context, zmq::socket_type::push);
    push->connect("tcp://localhost:5555");

    sub = new zmq::socket_t(context, zmq::socket_type::sub);
    sub->connect("tcp://localhost:5556");
    sub->setsockopt(ZMQ_SUBSCRIBE, "", 0); // subscribe to all
}

void shutdownNetwork() {
    delete push;
    delete sub;
}

void sendVel(int playerId, int velx, int vely) {
    std::string msg = std::to_string(playerId) + " " + std::to_string(velx) + " " + std::to_string(vely);
    zmq::message_t message(msg.size());
    memcpy(message.data(), msg.data(), msg.size());
    push->send(message, zmq::send_flags::none);
}

bool update(int& playerId, int& velx, int& vely) {
    zmq::message_t msg;
    zmq::recv_result_t result = sub->recv(msg, zmq::recv_flags::dontwait);
    if (!result) {
        return false;
    }

    std::string update(static_cast<char*>(msg.data()), msg.size());
    sscanf(update.c_str(), "%d %d %d", &playerId, &velx, &vely);
    return true;
}