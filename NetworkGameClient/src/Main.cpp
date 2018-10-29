#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time.hpp>
#include <boost/beast/core.hpp>
#include <chrono>

void timer_expired(std::string id)
{
	std::cout << boost::posix_time::second_clock::local_time() << " " << id << " enter." << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << boost::posix_time::second_clock::local_time() << " " << id << " leave." << std::endl;
}

class printer
{
public:
	printer(boost::asio::io_context& io)
		: m_timer(io, boost::asio::chrono::seconds(1)),
		m_count(0)
	{
		m_timer.async_wait(boost::bind(&printer::print, this));
	}
	~printer()
	{
		std::cout << "The final count was: " << m_count << std::endl;
	}

	void print()
	{
		if (m_count < 5)
		{
			std::cout << m_count << std::endl;
			++m_count;

			m_timer.expires_at(m_timer.expiry() + boost::asio::chrono::seconds(1));
			m_timer.async_wait(boost::bind(&printer::print, this));
		}
	}
private:
	boost::asio::steady_timer m_timer;
	int m_count;

};
/*
void print(const boost::system::error_code& /*e,
	boost::asio::steady_timer* t, int* count)
{
	if (*count < 5)
	{
		std::cout << "Hello, world! " << *count << std::endl;
		++(*count);
		t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));

		t->async_wait(boost::bind(print,
			boost::asio::placeholders::error, t, count));
	}
}
*/

int main(int argc, void** argv)
{
	using namespace boost;

	asio::io_service service;
	asio::io_service::strand strand(service);

	asio::deadline_timer timer1(service, posix_time::seconds(5));
	asio::deadline_timer timer2(service, posix_time::seconds(5));
	asio::deadline_timer timer3(service, posix_time::seconds(6));

	timer1.async_wait(
	strand.wrap([](auto ... vn) { timer_expired("timer1"); })
	);

	timer2.async_wait(
	strand.wrap([](auto ... vn) { timer_expired("timer2"); })
	);

	timer3.async_wait([](auto ... vn) { timer_expired("timer3"); });

	std::thread ta([&]() {service.run(); } );
	std::thread tb([&]() {service.run(); });

	ta.join();
	tb.join();

	std::cout << "done." << std::endl;

	std::cin.get();
	return 0;
}