//  Clone server Model One

#include "zmq.hpp"
#include "zmq_addon.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <string>

int main(void)
{
    //  Prepare our context and publisher socket
    zmq::context_t ctx(1);

    zmq::socket_t publisher(ctx, zmq::socket_type::pub);
    publisher.bind("tcp://*:5556");
    
    zmq::socket_t command_receiver(ctx, zmq::socket_type::sub);
    command_receiver.bind("tcp://*:5557");
    command_receiver.set(zmq::sockopt::subscribe, "");

    uint64_t wahoo = 0;
    std::cout << "start done" << std::endl;

    while (true)
    {
        zmq::message_t received;
        command_receiver.recv(received, zmq::recv_flags::none);

        std::cout << "received: \"" << received.to_string() << "\"" << std::endl;

        std::string msg_string = "nice, thank you bro, i got this from you \"" + received.to_string() + "\"";

        zmq::message_t msg(msg_string.c_str(), msg_string.length() + 1);
        publisher.send(msg, zmq::send_flags::none);

        wahoo++;
    }
    return 0;
}