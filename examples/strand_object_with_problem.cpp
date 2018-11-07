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
boost::mutex g_coutMutex;
const unsigned int cg_numberOfThreads = 2;

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

void PrintNum(int p_value)
{
	//g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: x = " 
		<< p_value << std::endl;
	//g_coutMutex.unlock();
}

int main(int argc, char* argv[])
{
	/* create io_service, work objects and strand object*/
	std::shared_ptr<boost::asio::io_service> l_ioService(
		new boost::asio::io_service
	);
	std::shared_ptr<boost::asio::io_service::work> l_work(
		new boost::asio::io_service::work( *l_ioService)
	);
	boost::asio::io_service::strand l_strand (*l_ioService);

	/* Program will exit, once work is finished */

	// create worker threads, so ioService has something to do.
	boost::thread_group l_workerThreads;
	for (int i = 0; i < cg_numberOfThreads; ++i)
	{
		l_workerThreads.create_thread(boost::bind( &WorkerThread, l_ioService));
	}

	// as we are not going through strand object api, the order of execution isn't guaranteed.
	boost::this_thread::sleep( boost::posix_time::milliseconds(100));
	l_ioService->post( l_strand.wrap( boost::bind(&PrintNum, 1 ) ) );
	l_ioService->post( l_strand.wrap( boost::bind(&PrintNum, 2 ) ) );

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	l_ioService->post( l_strand.wrap( boost::bind(&PrintNum, 3 ) ) );
	l_ioService->post( l_strand.wrap( boost::bind(&PrintNum, 4 ) ) );

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	l_ioService->post( l_strand.wrap( boost::bind(&PrintNum, 5 ) ) );
	l_ioService->post( l_strand.wrap( boost::bind(&PrintNum, 6 ) ) );



	l_work.reset();

	l_workerThreads.join_all();

	// no need to lock, as all threads have been joined.
	std::cout << "Press Enter to exit." << std::endl;
	std::cin.get();

	return 0;
}