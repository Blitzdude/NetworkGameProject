#pragma once
#include <memory>
#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "tagOrDie.h"

enum class ClientMessage : uint8
{
    Join,       // tell server we're new here
    Leave,      // tell server we're leaving
    Input       // tell server our user input
};

enum class ServerMessage : uint8
{
    Accept,     // tell client they're accepted
    Denied,     // tell client they're rejected
    State       // tell client the game state
};

/* networkManager class manages the io_context object and the lifetime of the connection*/
class NetworkManager
{
public:
    boost::asio::io_context m_ioContext;
private:
    std::shared_ptr<boost::asio::io_context::work> m_ptr_work;
};


class Server : public boost::enable_shared_from_this<Server>
{
// Methods
private:

public:
    Server();
    virtual ~Server();

    bool HasStopped();
    void Run();
    void Stop();
    
    void HandleSend(const boost::system::error_code p_ec, std::size_t p_bytesTransferred);
    void HandleReceive(const boost::system::error_code p_ec, std::size_t p_bytesTransferred);
    // Getters

    // Members
private:
    NetworkManager m_networkManager;

    boost::asio::ip::udp::socket m_socket;
    std::shared_ptr<boost::asio::io_service::work> m_ptr_work;
    boost::asio::ip::udp::endpoint m_endpoints[TOD_MAX_CLIENTS];

    
    uint32 m_shutdown; // mark as volatile for atomic checks?
};


// -----------------------------------------------------------------------------------------
class Client : public boost::enable_shared_from_this<Client>
{
public:
    Client();
    
    void Send();
    void JoinServer();
    void Disconnect();

    void HandleConnect(const boost::system::error_code & p_ec, boost::asio::ip::udp::resolver::iterator p_iterator);
    void HandleReceive(const boost::system::error_code& p_ec, size_t p_bytesTransferred, );

// Members
private:
    NetworkManager m_networkManager;
    boost::asio::ip::udp::socket m_socket; // move to initializer list
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
};
