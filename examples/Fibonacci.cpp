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
/*
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
*/

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

size_t Fibonacci(size_t p_value)
{
	if (p_value <= 1)
	{
		return p_value;
	}
	boost::this_thread::sleep(
		boost::posix_time::milliseconds( 10 )
	);
	return Fibonacci(p_value - 1) + Fibonacci(p_value - 2);
}

void CalculateFibonacci(size_t p_value)
{
	g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: Now Calculating Fibonacci:( " 
		<< p_value << ")" << std::endl;
	g_coutMutex.unlock();

	size_t l_fib = Fibonacci(p_value);

	g_coutMutex.lock();
	std::cout << "[" << boost::this_thread::get_id() << "]: Fibonacci:( "
		<< p_value << ") = " << l_fib << std::endl;
	g_coutMutex.unlock();

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
	
	boost::thread_group l_workerThreads;
	for (int i = 0; i < cg_numberOfThreads; ++i)
	{
		l_workerThreads.create_thread(boost::bind( &WorkerThread, l_ioService));
	}

	l_ioService->post( boost::bind(	CalculateFibonacci, 12));
	l_ioService->post( boost::bind(	CalculateFibonacci, 13));
	l_ioService->post( boost::bind( CalculateFibonacci, 14));

	l_work.reset();

	l_workerThreads.join_all();

	std::cout << "Press Enter to exit." << std::endl;
	std::cin.get();

	return 0;
}