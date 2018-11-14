#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <memory>


// TODO: Finish going through tutorial TCP
/* coding convention
variables:
    l_ = local
    p_ = parameter
    m_ = member
    g_ = global
    *_ptr_ = pointer
    c(*)_ = constant
    s(*)_ = static
    sc(*)_ = static constant
    

Naming:
    variables: m_fooBar
    Functions: FooBar()

*******************/

// GLOBALS
boost::mutex g_coutMutex;
const unsigned int cg_numberOfThreads = 2;

/* basic printing function with locking for threads */
void TPrint(std::string p_msg)
{
    g_coutMutex.lock();
    std::cout << "[" << boost::this_thread::get_id() << "] " << p_msg << std::endl;
    g_coutMutex.unlock();
}


void TPrintError(const boost::system::error_code &p_ec)
{
    g_coutMutex.lock();
    std::cout << "[" << boost::this_thread::get_id() << "] Error: " << p_ec << std::endl;
    g_coutMutex.unlock();
}

void TPrintException(const std::exception &p_ex)
{
    g_coutMutex.lock();
    std::cout << "[" << boost::this_thread::get_id() << "] Exception: " << p_ex.what() << std::endl;
    g_coutMutex.unlock();
}


/************************
What does boost::asio UDP server need?
- io_service object -- abstracts OS interfaces that process data asynchronously.
- udp_socket -- Where to listen to for connections
- udp_endpoint -- Information about sender

io_service looks towards Operating System (OS) API calls.
io_objects look towards tasks designated by programmer (Eg. work object)
*************************/
class Server
{
public:
    Server();
    ~Server();

    void Receive();
    void Send();
    void Stop();
    
    // Getters
    std::shared_ptr<boost::asio::io_service> GetIOService() { return m_ptr_ioService;};
    std::shared_ptr<boost::asio::ip::udp::socket> GetSocket() {return m_ptr_socket;};
    std::shared_ptr<boost::asio::ip::udp::endpoint> GetEndpoint() {return m_ptr_endpoint;};
    std::shared_ptr<boost::asio::io_service::strand> GetStrand() { return m_ptr_strand;};



private:
    std::shared_ptr<boost::asio::io_service> m_ptr_ioService;
    std::shared_ptr<boost::asio::ip::udp::socket> m_ptr_socket;
    std::shared_ptr<boost::asio::ip::udp::endpoint> m_ptr_endpoint;
    std::shared_ptr<boost::asio::io_service::strand> m_ptr_strand;
    std::shared_ptr<boost::asio::io_service::work> m_ptr_work;
};

Server::Server()
    : m_ptr_ioService(new boost::asio::io_service)
    , m_ptr_work(new boost::asio::io_service::work(*m_ptr_ioService))
    , m_ptr_strand(new boost::asio::io_service::strand(*m_ptr_ioService))
{

}

Server::~Server()
{
    Stop();
}

void Server::Receive()
{

}

void Server::Stop()
{
    boost::system::error_code l_ec;
    m_ptr_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, l_ec);
    m_ptr_socket->close(l_ec);
    if (l_ec)
    {
        TPrintError(l_ec);
    }
    m_ptr_ioService->stop();
}


/* Simple thread writing to cout*/
void WorkerThread(std::shared_ptr<boost::asio::io_service> p_ioService)
{
    TPrint("Starting:");

    while (true)
    {
        try
        {
            boost::system::error_code l_ec;
            p_ioService->run(l_ec);

            if (l_ec)
            {
                TPrintError(l_ec);
            }
            break;
        }
        catch (const std::exception& p_ex)
        {
            TPrintException(p_ex);
        }
    }

    TPrint("Exiting");
}

void OnAccept(const boost::system::error_code& p_ec, std::shared_ptr<boost::asio::ip::tcp::socket> p_socket)
{
    if (p_ec)
    {
        TPrintError(p_ec);
    }
    else
    {
        TPrint("Accepted!");
    }
}

int mainNot(int argc, char* argv[])
{
/* // moved to Server class constructor
    // create io_service, work objects and strand object
    std::shared_ptr<boost::asio::io_service> l_ioService(
        new boost::asio::io_service
    );
    // Does the actual work --> functions
    std::shared_ptr<boost::asio::io_service::work> l_work(
        new boost::asio::io_service::work(*l_ioService)
    );
    // Functions passed through strand are resolved in correct order
    std::shared_ptr<boost::asio::io_service::strand> l_strand(
        new boost::asio::io_service::strand(*l_ioService)
    );
*/
    Server l_server;
    TPrint("Press Enter the exit.");

    // create worker threads, so ioService has something to do.
    boost::thread_group l_workerThreads;
    for (int i = 0; i < cg_numberOfThreads; ++i)
    {
        l_workerThreads.create_thread(boost::bind(&WorkerThread, l_server.GetIOService()) );
    }

    // Accepts connections
    /*  NOT NEEDED IN UDP
    std::shared_ptr<boost::asio::ip::tcp::acceptor> l_acceptor(
        new boost::asio::ip::tcp::acceptor(* l_ioService)
    );

    // connection goes through this socket
    std::shared_ptr<boost::asio::ip::tcp::socket> l_socket(
        new boost::asio::ip::tcp::socket(*l_ioService)
    );
    */

    try
    {
        boost::asio::ip::tcp::resolver::query l_query(
            "127.0.0.1", // Local host
            boost::lexical_cast<std::string>(7777)
        );
       /// boost::asio::ip::tcp::endpoint l_endpoint = *l_resolver.resolve(l_query);
        /*
        l_acceptor->open( l_endpoint.protocol());
        l_acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
        l_acceptor->bind(l_endpoint);
        l_acceptor->listen( boost::asio::socket_base::max_connections);
        l_acceptor->async_accept(*l_socket, boost::bind( OnAccept, _1, l_socket));
        */

        g_coutMutex.lock();
       /// std::cout << "Listening on: " << l_endpoint << std::endl;
        g_coutMutex.unlock();

    }
    catch (const std::exception& l_ex)
    {
        TPrintException(l_ex);
    }

    std::cin.get();

    boost::system::error_code l_ec;

    l_server.Stop();

    l_workerThreads.join_all();


    return 0;
}