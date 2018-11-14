
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "tagOrDie.h"

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class server
{
public:
    server(boost::asio::io_service& io_service, int16 port)
        : socket_(io_service, udp::endpoint(udp::v4(), port))
    {
        do_receive();
    }

    void do_receive()
    {
        socket_.async_receive_from(
            boost::asio::buffer(data_, SOCKET_BUFFER_SIZE), sender_endpoint_,
            [this](boost::system::error_code ec, uint32 bytes_recvd)
        {
            if (!ec && bytes_recvd > 0)
            {
                std::cout << "Bytes received: " << bytes_recvd << " " << data_ <<  std::endl;
                do_send(bytes_recvd);
            }
            else
            {
                do_receive();
            }
        });
    }

    void do_send(uint32 length)
    {
        socket_.async_send_to(
            boost::asio::buffer(data_, length), sender_endpoint_,
            [this](boost::system::error_code /*ec*/, uint32 /*bytes_sent*/)
        {
            do_receive();
        });
    }

private:
    udp::socket socket_;
    udp::endpoint sender_endpoint_;
    int8 data_[SOCKET_BUFFER_SIZE];
};

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_service io_service;

        server s(io_service, PORT);

        io_service.run();

        // Player init
        int32 l_playerX = 0;
        int32 l_playerY = 0;

        bool32 l_isRunning = 1;

        while (l_isRunning)
        {

        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}