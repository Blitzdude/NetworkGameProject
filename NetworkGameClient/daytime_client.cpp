//
// blocking_udp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "TagOrDie.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

using boost::asio::ip::udp;


int main(int argc, char* argv[])
{

    try
    {
        boost::asio::io_context io_context;

        udp::socket s(io_context, udp::endpoint(udp::v4(), 0));

        udp::resolver resolver(io_context);
        udp::resolver::results_type endpoints =
            resolver.resolve(udp::v4(), "127.0.0.1", boost::lexical_cast<std::string>(PORT) );

        // Player init
        int8 l_buffer[SOCKET_BUFFER_SIZE];
        int32 l_playerX = 0;
        int32 l_playerY = 0;

        bool32 l_isRunning = 1;


        std::cout << "Type w, a, s, d to move, q to quit" << std::endl;
        while (l_isRunning)
        {
            // get input
            std::cout << "Enter message: ";
            std::cin >> l_buffer[0];
           
            // send to server


            int8 request[SOCKET_BUFFER_SIZE];
            std::cin.getline(request, SOCKET_BUFFER_SIZE);
            size_t request_length = std::strlen(request);
            s.send_to(boost::asio::buffer(request, request_length), *endpoints.begin());

            // wait for reply
            int8 reply[SOCKET_BUFFER_SIZE];
            udp::endpoint sender_endpoint;
            size_t reply_length = s.receive_from(
                boost::asio::buffer(reply, SOCKET_BUFFER_SIZE), sender_endpoint);
            std::cout << "Reply is: ";
            std::cout.write(reply, reply_length);
            std::cout << "\n";
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    std::cin.get();
    return 0;
}