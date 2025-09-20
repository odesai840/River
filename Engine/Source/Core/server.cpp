#include <zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include "server.h"

void RiverCore::server::reply() {
    zmq::context_t context(1);
    zmq::socket_t responder(context, zmq::socket_type::rep);
    responder.bind("tcp://*:5555");

    responder.close();
    context.close();

}