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
const unsigned int cg_numberOfThreads = 4;

/// not used
class MyClass
{
public:
	void Function3(int p_i, float p_f)
	{
		std::cout << "i: " << p_i << std::endl;
		std::cout << "f: " << p_f << std::endl;
	}

private:

};

void Function1()
{
	std::cout << __FUNCTION__ << std::endl;
}

void Function2(int p_i, float p_f)
{
	std::cout << "i: " << p_i << std::endl;
	std::cout << "f: " << p_f << std::endl;
}

void ThreadPrint(std::string p_msg)
{
	g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: " << p_msg << std::endl;
	g_coutMutex.unlock();
}

/* Simple thread writing to cout*/
void WorkerThread(std::shared_ptr<boost::asio::io_service> p_ioService)
{
	ThreadPrint("Thread Start");
	p_ioService->run();
	ThreadPrint("Thread Exit");
	
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
	
	ThreadPrint(" Press [return] to exit.");

	boost::thread_group l_workerThreads;
	for (int i = 0; i < cg_numberOfThreads; ++i)
	{
		l_workerThreads.create_thread(boost::bind( &WorkerThread, l_ioService));
	}

	std::cin.get();

	l_ioService->stop();
	l_workerThreads.join_all();

	return 0;
}