#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <string>
#include <memory>

// TODO: Continue working on asio

/* coding conventions
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
const unsigned int cg_numberOfThreads = 1;

/* Simple thread writing to cout*/
void WorkerThread(std::shared_ptr<boost::asio::io_service> p_ioService)
{
	g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: Starting:" << std::endl;
	g_coutMutex.unlock();

	p_ioService->run();

	g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: Exiting: " << std::endl;
	g_coutMutex.unlock();
}

void Dispatch(int p_value)
{
	g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: "  << __FUNCTION__ 
		" x = " << p_value << std::endl;
	g_coutMutex.unlock();
}

void Post(int p_value)
{
	g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: " << __FUNCTION__
		" x = " << p_value << std::endl;
	g_coutMutex.unlock();
}

void Run3(std::shared_ptr< boost::asio::io_service> p_ioService)
{
	for (int i = 0; i < 3; ++i)
	{
		p_ioService->dispatch( boost::bind( &Dispatch, i * 2));
		p_ioService->post(boost::bind( &Post, i * 2 + 1) );
		boost::this_thread::sleep( boost::posix_time::milliseconds( 1000));
	}
}

int main(int argc, char* argv[])
{
	/* create io_service and work objects*/
	std::shared_ptr<boost::asio::io_service> l_ioService(
		new boost::asio::io_service
	);
	std::shared_ptr<boost::asio::io_service::work> l_work(
		new boost::asio::io_service::work( *l_ioService)
	);
	
	/* Program will exit, once work is finished */

	// create worker threads, so ioService has something to do.
	boost::thread_group l_workerThreads;
	for (int i = 0; i < cg_numberOfThreads; ++i)
	{
		l_workerThreads.create_thread(boost::bind( &WorkerThread, l_ioService));
	}

	l_ioService->post( boost::bind(	&Run3, l_ioService));

	l_work.reset();

	l_workerThreads.join_all();

	// no need to lock, as all threads have been joined.
	std::cout << "Press Enter to exit." << std::endl;
	std::cin.get();

	return 0;
}