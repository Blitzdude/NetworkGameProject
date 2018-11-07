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

void TimerHandler(const boost::system::error_code& p_ec,
					std::shared_ptr<boost::asio::deadline_timer> p_timer)
{
	if (p_ec)
	{
		TPrintError(p_ec);
	}
	else
	{
		TPrint("TimeHandler");
		p_timer->expires_from_now(boost::posix_time::seconds(5));
		p_timer->async_wait(boost::bind( &TimerHandler, _1, p_timer));
	}
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

	TPrint("This program will exit when all work has finished");

	// create worker threads, so ioService has something to do.
	boost::thread_group l_workerThreads;
	for (int i = 0; i < cg_numberOfThreads; ++i)
	{
		l_workerThreads.create_thread(boost::bind( &WorkerThread, l_ioService));
	}

	/* Create an asynchronous 5 second Deadline timer shared pointer */
	/* NOTE: in async_wait() function, the _1 parameter is a placeholder parameter.
		Basically it means "First parameter, which will be supplied later.*/

	std::shared_ptr<boost::asio::deadline_timer> l_timer(
		new boost::asio::deadline_timer( *l_ioService)
	);
	l_timer->expires_from_now(boost::posix_time::seconds(5));
	l_timer->async_wait(boost::bind( &TimerHandler, _1, l_timer));

	std::cin.get();

	l_ioService->stop();

	l_workerThreads.join_all();

	TPrint("Press Enter to exit program");
	std::cin.get();


	return 0;
}
