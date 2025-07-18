//  Clone client Model One

#include "zmq.hpp"
#include "zmq_addon.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(void)
{
    //  Prepare our context and updates socket
    zmq::context_t ctx(1);

    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://localhost:5556");
    subscriber.set(zmq::sockopt::subscribe, "");

    zmq::socket_t command_sender(ctx, zmq::socket_type::pub);
    command_sender.connect("tcp://localhost:5557");

    std::cout << "start done" << std::endl;

    while (true)
    {
        zmq::message_t received;
        while(subscriber.recv(received, zmq::recv_flags::dontwait)) {
            std::cout << "received: \"" << received.to_string() << "\"" << std::endl;
        }
    }
    return 0;
}