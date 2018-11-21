#include "NetworkManager.h"
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

NetworkManager::NetworkManager()
    : m_ptr_work(new boost::asio::io_context::work(m_ioContext))
{
}

NetworkManager::~NetworkManager()
{
}

Server::Server()
    : m_socket(m_networkManager.m_ioContext, boost::asio::ip::udp::v4(), TOD_PORT)
{

}

Server::~Server()
{
}

Client::Client()
    : m_socket(m_networkManager.m_ioContext)
{
    try
    {
        // have the resolver resolve a query for the server
        boost::asio::ip::udp::resolver l_resolver(m_networkManager.m_ioContext);
        boost::asio::ip::udp::resolver::query l_query("127.0.0.1", boost::lexical_cast<std::string>(TOD_PORT));
        boost::asio::ip::udp::resolver::iterator l_iterator = l_resolver.resolve(l_query);

        m_remoteEndpoint = *l_iterator;

        // open socket and start sending data
        m_socket.async_connect(m_remoteEndpoint,
            boost::bind(&Client::handleConnect, this)
    }
    catch (const std::exception& p_ex)
    {
        std::cerr << p_ex.what() << std::endl;
    }
}

void Client::HandleConnect(const boost::system::error_code & p_ec, boost::asio::ip::udp::resolver::iterator p_iterator)
{
    if (!p_ec)
    { // Succesfully connected
        m_socket.async_receive(boost::asio)
    }
    else if (p_iterator != boost::asio::ip::udp::resolver::iterator())
    { // if there are connections to check
        m_socket.close();
        boost::asio::ip::udp::endpoint l_endpoint = *p_iterator;
        // 

        m_socket.async_connect()

    }
}

void Client::HandleReceive(const boost::system::error_code& p_ec, size_t p_bytesTransferred)
{
    // TODO: what to do with buffer?
}