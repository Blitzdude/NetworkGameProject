#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <string>
#include <memory>


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
boost::asio::io_service g_ioService;
boost::mutex g_coutMutex;
const unsigned int g_numberOfThreads = 4;



/* Simple thread writing to cout*/
void WorkerThread()
{
	g_coutMutex.lock();
	std::cout << "Thread start:" << std::endl;
	g_coutMutex.unlock();

	g_ioService.run();

	g_coutMutex.lock();
	std::cout << "Thread exit:" << std::endl;
	g_coutMutex.unlock();

}

int main(int argc, char* argv[])
{

	// in order for us to remove work from io_service, we must make it a smart pointer 
	std::shared_ptr<boost::asio::io_service::work> l_work(
		new boost::asio::io_service::work(g_ioService)
	);

	std::cout << "Press Enter to exit" << std::endl;

	// boost::thread_group manages threads for us
	boost::thread_group l_workerThreads;
	for (unsigned int i = 0; i < g_numberOfThreads; ++i)
	{
		l_workerThreads.create_thread( WorkerThread );
	}

	std::cin.get();

	g_ioService.stop();

	l_workerThreads.join_all();

	std::cin.get();

	return 0;
}