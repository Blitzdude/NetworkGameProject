//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

//using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
//namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

//------------------------------------------------------------------------------

// Report a failure
void Fail(boost::system::error_code p_ec, const char* p_what)
{
	std::cerr << p_what << ": " << p_ec.message() << "\n";
}

// Performs an HTTP GET and prints the response
class Session : public std::enable_shared_from_this<Session>
{
	boost::asio::ip::tcp::resolver m_resolver;
	boost::asio::ip::tcp::socket m_socket;
	boost::beast::flat_buffer m_buffer; // (Must persist between reads)
	boost::beast::http::request<boost::beast::http::empty_body> m_req;
	boost::beast::http::response<boost::beast::http::string_body> m_res;

public:
	// Resolver and socket require an io_context
	explicit Session(boost::asio::io_context& ioc)
		: m_resolver(ioc)
		, m_socket(ioc)
	{
	}

	// Start the asynchronous operation
	void Run(char const* host,
			char const* port,
			char const* target,
			int version)
	{
		// Set up an HTTP GET request message
		m_req.version(version);
		m_req.method(boost::beast::http::verb::get);
		m_req.target(target);
		m_req.set(boost::beast::http::field::host, host);
		m_req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		// Look up the domain name
		m_resolver.async_resolve(
			host,
			port,
			std::bind(
				&Session::OnResolve,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void OnResolve(boost::system::error_code p_ec,
			boost::asio::ip::tcp::resolver::results_type p_results)
	{
		if (p_ec)
		{
			return Fail(p_ec, "resolve");
		}

		// Make the connection on the IP address we get from a lookup
		boost::asio::async_connect(
			m_socket,
			p_results.begin(),
			p_results.end(),
			std::bind(
				&Session::OnConnect,
				shared_from_this(),
				std::placeholders::_1));
	}

	void OnConnect(boost::system::error_code p_ec)
	{
		if (p_ec)
		{
			return Fail(p_ec, "connect");
		}

		// Send the HTTP request to the remote host
		boost::beast::http::async_write(m_socket, m_req,
			std::bind(
				&Session::OnWrite,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void OnWrite(boost::system::error_code p_ec,
			std::size_t p_bytes_transferred)
	{
		boost::ignore_unused(p_bytes_transferred);

		if (p_ec)
		{
			return Fail(p_ec, "write");
		}

		// Receive the HTTP response
		boost::beast::http::async_read(m_socket, m_buffer, m_res,
			std::bind(
				&Session::OnRead,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void OnRead( boost::system::error_code p_ec,
			std::size_t p_bytes_transferred)
	{
		boost::ignore_unused(p_bytes_transferred);

		if (p_ec)
		{
			return Fail(p_ec, "read");
		}
		else
		{
			// Write the message to standard out
			std::cout << m_res << std::endl;

			// Gracefully close the socket
			m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, p_ec);

			// not_connected happens sometimes so don't bother reporting it.
			if (p_ec && p_ec != boost::system::errc::not_connected)
			{
				return Fail(p_ec, "shutdown");
			}
		}

		// If we get here then the connection is closed gracefully
	}
};

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
	// Check command line arguments.
	if (argc != 4 && argc != 5)
	{
		std::cerr <<
			"Usage: http-client-async <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
			"Example:\n" <<
			"    http-client-async www.example.com 80 /\n" <<
			"    http-client-async www.example.com 80 / 1.0\n";
	}
	else
	{
		auto const host = argv[1];
		auto const port = argv[2];
		auto const target = argv[3];
		int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

		// The io_context is required for all I/O
		boost::asio::io_context l_IOContext;

		// Launch the asynchronous operation
		std::make_shared<Session>(l_IOContext)->Run(host, port, target, version);

		// Run the I/O service. The call will return when
		// the get operation is complete.
		l_IOContext.run();

		std::cin.get();
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;

}
