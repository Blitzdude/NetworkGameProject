#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

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
	boost::asio::io_context io;
	printer p(io);
	io.run();

	return 0;
}