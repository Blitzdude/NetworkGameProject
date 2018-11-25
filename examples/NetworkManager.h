#pragma once
#include <memory>
#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/regex.hpp>

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

// typedef std::deque<std::shared_ptr<boost::asio::ip::udp::socket>> ios_deque;

/**
 * Connection class, implementing async input/output
 *
 */
class connection : public boost::enable_shared_from_this<connection> {
public:
    typedef std::shared_ptr<connection> pointer;

    /**
     * Create new connection
     *
     * @param io_service io_service in which this connection will work
     *
     * @return pointer to newly allocated object
     */
    static std::shared_ptr<connection> CreateConnection(boost::asio::io_service& io_service) {
        return std::shared_ptr<connection>(new connection(io_service));
    }

    /**
     * Return socket, associated with this connection. This socket used in accept operation
     *
     *
     * @return reference to socket
     */
    boost::asio::ip::udp::socket& GetSocket() {
        return m_socket;
    }

    /**
     * Start input/output chain with reading of headers from browser
     *
     */
    void start() {
        // start reading of headers from browser
        boost::asio::async_read_until(m_socket, buf, boost::regex("\r\n\r\n"),
            boost::bind(&connection::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

private:
    /**
     * Initialize connection
     *
     * @param io_service
     *
     * @return
     */
    connection(boost::asio::io_service& io_service) : m_socket(io_service) {
    }

    /**
     * Called when data written to browser
     *
     * @param error object, containing information about errors
     * @param bytes_transferred number of transferred bytes
     */
    void handle_write(const boost::system::error_code& error,
        size_t bytes_transferred) {
    }

    /**
     * Called when data readed from browser
     *
     * @param error object, containing information about errors
     * @param bytes_transferred number of transferred bytes
     */
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred) {
        boost::asio::async_write(m_socket, boost::asio::buffer(message_),
            boost::bind(&connection::handle_write, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    boost::asio::ip::udp::socket m_socket; /**< socket, associated with browser */
    boost::asio::streambuf buf;  /**< buffer for request data */
    static std::string message_; /**< data, that we'll return to browser */
};

std::string connection::message_ = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<html><head><title>test</title>"
"</head><body><h1>Test</h1><p>This is a test!</p></body></html>";


/**
 * Server class
 *
 */
class Server {
public:
    /**
     * Initialize all needed data
     *
     * @param io_service reference to io_service
     * @param port port to listen on, by default - 10001
     */
    Server(const std::shared_ptr<boost::asio::io_context>& p_ioContext, int p_port = 10001)
        : m_ioContext(p_ioContext)
     //   , m_acceptor(*io_services.front(), boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)) 
    {
        StartAccept();
    }

private:
    /**
     * start connection accepting in async mode
     *
     */
    void StartAccept() {
        // select next io_service object
        // m_ioServices.push_back(m_ioServices.front());
        // m_ioServices.pop_front();
        // create new connection
        connection::pointer new_connection = connection::CreateConnection(*m_ioContext);
        // start acceptor in async mode
        /*
        m_acceptor.async_accept(new_connection->m_socket(),
            boost::bind(&server::HandleAccept, this, new_connection,
                boost::asio::placeholders::error));
        */
    }

    /**
     * Run when new connection is accepted
     *
     * @param new_connection accepted connection
     * @param error reference to error object
     */
    void HandleAccept(connection::pointer new_connection,
        const boost::system::error_code& error) {
        if (!error) {
            new_connection->start();
            StartAccept();
        }
    }

    std::shared_ptr<boost::asio::io_context> m_ioContext;
    // std::deque<std::shared_ptr<boost::asio::io_context>> m_ioContexts;	/**< deque of pointers to io_services */
    
    // boost::asio::ip::udp::acceptor m_acceptor; /**< object, that accepts new connections */
};


class Client
{
public:
    Client(
        boost::asio::io_context& p_ioContext,
        const std::string& p_host,
        const std::string& p_port
    ) 
        : m_ioContext(p_ioContext)
        , m_socket(p_ioContext, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0))
    {
        boost::asio::ip::udp::resolver l_resolver(m_ioContext);
        boost::asio::ip::udp::resolver::query l_query(boost::asio::ip::udp::v4(), p_host, p_port);
        boost::asio::ip::udp::resolver::iterator l_iter = l_resolver.resolve(l_query);
        m_endpoint = *l_iter;
    }

    ~Client()
    {
        m_socket.close();
    }

    void Send(const std::string& p_msg) {
        m_socket.send_to(boost::asio::buffer(p_msg, p_msg.size()), m_endpoint);
    }

private:
    boost::asio::io_context& m_ioContext;
    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_endpoint;
};

