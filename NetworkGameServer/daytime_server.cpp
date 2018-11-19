
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
        : m_socket(io_service, udp::endpoint(udp::v4(), port))
    {
        DoReceive();
    }

    void DoReceive()
    {
        m_socket.async_receive_from(
            boost::asio::buffer(m_data, SOCKET_BUFFER_SIZE), m_senderEndpoint,
            [this](boost::system::error_code p_ec, uint32 p_bytesReovered)
        {
            if (!p_ec && p_bytesReovered > 0)
            {
                std::cout << "Bytes received: " << p_bytesReovered << " " << m_data <<  std::endl;
                DoSend(p_bytesReovered);
            }
            else
            {
                DoReceive();
            }
        });
    }

    void DoSend(uint32 length)
    {
        // TODO: This no work
        // compose message
        int32 l_writeIndex = 0;
        int32 l_buffer[SOCKET_BUFFER_SIZE];
        
        // Send back to client
        m_socket.async_send_to(
            boost::asio::buffer(l_buffer, SOCKET_BUFFER_SIZE), m_senderEndpoint,
            [this](boost::system::error_code /*ec*/, uint32 /*bytes_sent*/)
        {
            DoReceive();
        });
    }

    // Getters
    const int8& GetData() const { return m_data[0];   };

    // Player init

    bool32 l_isRunning = 1;
    int32 l_playerX = 0;
    int32 l_playerY = 0;

private:
    udp::socket m_socket;
    udp::endpoint m_senderEndpoint;
    int8 m_data[SOCKET_BUFFER_SIZE];



};

int notmain1(int argc, char* argv[])
{
    try
    {
        boost::asio::io_service io_service;

        server l_server(io_service, PORT);

        io_service.run();


        while (l_server.l_isRunning)
        {
            int8 player_input = l_server.GetData();
            switch (player_input)
            {
            case 'w':
                ++l_server.l_playerY;
                break;
            case 's':
                --l_server.l_playerY;
                break;
            case 'a':
                --l_server.l_playerX;
                break;
            case 'd':
                ++l_server.l_playerX;
                break;
            case 'q':
                l_server.l_isRunning = 0;
                break;
            default:
                std::cout << "unhandled input" << std::endl;
                break;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}