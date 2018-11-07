#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <string>
#include <memory>


/* coding convention
variables:
	l_ = local
	p_ = parameter
	m_ = member
	g_ = global
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

void TPrintError( const boost::system::error_code &p_ec)
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
			TPrintException( p_ex );
		}
	}

	TPrint("Exiting");
}

/* Recursivly throws an runtime error over and over. */
void RaiseAnException(std::shared_ptr<boost::asio::io_service> p_ioService)
{
	TPrint(__FUNCTION__);

	p_ioService->post( boost::bind ( &RaiseAnException, p_ioService));

	throw(std::runtime_error( "Oops!"));
}

int main(int argc, char* argv[])
{
	/* create io_service, work objects*/
	std::shared_ptr<boost::asio::io_service> l_ioService(
		new boost::asio::io_service
	);
	std::shared_ptr<boost::asio::io_service::work> l_work(
		new boost::asio::io_service::work( *l_ioService)
	);

	TPrint("This program will exit when all work ahs finished");

	// create worker threads, so ioService has something to do.
	boost::thread_group l_workerThreads;
	for (int i = 0; i < cg_numberOfThreads; ++i)
	{
		l_workerThreads.create_thread(boost::bind( &WorkerThread, l_ioService));
	}

	l_ioService->post( boost::bind( &RaiseAnException, l_ioService));
	//l_work.reset();

	l_workerThreads.join_all();

	TPrint("Press Enter to exit program");
	std::cin.get();

	return 0;
}