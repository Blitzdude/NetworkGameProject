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
// Example: HTTP server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

//using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
//namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view GetMimeType(boost::beast::string_view p_path)
{
	using boost::beast::iequals;
	auto const cl_ext = [&p_path]
	{
		auto const pos = p_path.rfind(".");
		if (pos == boost::beast::string_view::npos)
		{
			return boost::beast::string_view{};
		}
		return p_path.substr(pos);
	}();
	if (iequals(cl_ext, ".htm"))  return "text/html";
	if (iequals(cl_ext, ".html")) return "text/html";
	if (iequals(cl_ext, ".php"))  return "text/html";
	if (iequals(cl_ext, ".css"))  return "text/css";
	if (iequals(cl_ext, ".txt"))  return "text/plain";
	if (iequals(cl_ext, ".js"))   return "application/javascript";
	if (iequals(cl_ext, ".json")) return "application/json";
	if (iequals(cl_ext, ".xml"))  return "application/xml";
	if (iequals(cl_ext, ".swf"))  return "application/x-shockwave-flash";
	if (iequals(cl_ext, ".flv"))  return "video/x-flv";
	if (iequals(cl_ext, ".png"))  return "image/png";
	if (iequals(cl_ext, ".jpe"))  return "image/jpeg";
	if (iequals(cl_ext, ".jpeg")) return "image/jpeg";
	if (iequals(cl_ext, ".jpg"))  return "image/jpeg";
	if (iequals(cl_ext, ".gif"))  return "image/gif";
	if (iequals(cl_ext, ".bmp"))  return "image/bmp";
	if (iequals(cl_ext, ".ico"))  return "image/vnd.microsoft.icon";
	if (iequals(cl_ext, ".tiff")) return "image/tiff";
	if (iequals(cl_ext, ".tif"))  return "image/tiff";
	if (iequals(cl_ext, ".svg"))  return "image/svg+xml";
	if (iequals(cl_ext, ".svgz")) return "image/svg+xml";
	return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(boost::beast::string_view p_base,
	boost::beast::string_view p_path)
{
	if (p_base.empty())
		return p_path.to_string();
	std::string l_result = p_base.to_string();
#if BOOST_MSVC
	char constexpr path_separator = '\\';
	if (l_result.back() == path_separator)
	{
		l_result.resize(l_result.size() - 1);
	}
	l_result.append(p_path.data(), p_path.size());

	for (auto& c : l_result)
	{
		if (c == '/')
		{
			c = path_separator;
		}
	}
#else
	char constexpr path_separator = '/';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
#endif
	return l_result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template< class Body, class Allocator, class Send>
void HandleRequest(
		boost::beast::string_view doc_root,
		boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& p_req,
		Send&& send)
{
	// Returns a bad request response
	auto const l_badRequest =
		[&p_req](boost::beast::string_view p_why)
	{
		boost::beast::http::response<boost::beast::http::string_body> l_res{ boost::beast::http::status::bad_request, p_req.version() };
		l_res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		l_res.set(boost::beast::http::field::content_type, "text/html");
		l_res.keep_alive(p_req.keep_alive());
		l_res.body() = p_why.to_string();
		l_res.prepare_payload();
		return res;
	};

	// Returns a not found response
	auto const not_found =
		[&req](boost::beast::string_view target)
	{
		boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::not_found, req.version() };
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "The resource '" + target.to_string() + "' was not found.";
		res.prepare_payload();
		return res;
	};

	// Returns a server error response
	auto const server_error =
		[&req](boost::beast::string_view what)
	{
		boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::internal_server_error, req.version() };
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "An error occurred: '" + what.to_string() + "'";
		res.prepare_payload();
		return res;
	};

	// Make sure we can handle the method
	if (req.method() != boost::beast::http::verb::get &&
		req.method() != boost::beast::http::verb::head)
		return send(bad_request("Unknown HTTP-method"));

	// Request path must be absolute and not contain "..".
	if (req.target().empty() ||
		req.target()[0] != '/' ||
		req.target().find("..") != boost::beast::string_view::npos)
		return send(bad_request("Illegal request-target"));

	// Build the path to the requested file
	std::string path = path_cat(doc_root, req.target());
	if (req.target().back() == '/')
		path.append("index.html");

	// Attempt to open the file
	boost::beast::error_code ec;
	boost::beast::http::file_body::value_type body;
	body.open(path.c_str(), boost::beast::file_mode::scan, ec);

	// Handle the case where the file doesn't exist
	if (ec == boost::system::errc::no_such_file_or_directory)
		return send(not_found(req.target()));

	// Handle an unknown error
	if (ec)
		return send(server_error(ec.message()));

	// Cache the size since we need it after the move
	auto const size = body.size();

	// Respond to HEAD request
	if (req.method() == boost::beast::http::verb::head)
	{
		boost::beast::http::response<boost::beast::http::empty_body> res{ boost::beast::http::status::ok, req.version() };
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, GetMimeType(path));
		res.content_length(size);
		res.keep_alive(req.keep_alive());
		return send(std::move(res));
	}

	// Respond to GET request
	boost::beast::http::response<boost::beast::http::file_body> res{
		std::piecewise_construct,
		std::make_tuple(std::move(body)),
		std::make_tuple(boost::beast::http::status::ok, req.version()) };
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, GetMimeType(path));
	res.content_length(size);
	res.keep_alive(req.keep_alive());
	return send(std::move(res));
}

//------------------------------------------------------------------------------

// Report a failure
void
fail(boost::system::error_code ec, char const* what)
{
	std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
	// This is the C++11 equivalent of a generic lambda.
	// The function object is used to send an HTTP message.
	struct send_lambda
	{
		session& self_;

		explicit
			send_lambda(session& self)
			: self_(self)
		{
		}

		template<bool isRequest, class Body, class Fields>
		void
			operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const
		{
			// The lifetime of the message has to extend
			// for the duration of the async operation so
			// we use a shared_ptr to manage it.
			auto sp = std::make_shared<
				boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

			// Store a type-erased version of the shared
			// pointer in the class to keep it alive.
			self_.res_ = sp;

			// Write the response
			boost::beast::http::async_write(
				self_.socket_,
				*sp,
				boost::asio::bind_executor(
					self_.strand_,
					std::bind(
						&session::on_write,
						self_.shared_from_this(),
						std::placeholders::_1,
						std::placeholders::_2,
						sp->need_eof())));
		}
	};

	boost::asio::ip::tcp::socket socket_;
	boost::asio::strand<
		boost::asio::io_context::executor_type> strand_;
	boost::beast::flat_buffer buffer_;
	std::shared_ptr<std::string const> doc_root_;
	boost::beast::http::request<boost::beast::http::string_body> req_;
	std::shared_ptr<void> res_;
	send_lambda lambda_;

public:
	// Take ownership of the socket
	explicit session(
			boost::asio::ip::tcp::socket socket,
			std::shared_ptr<std::string const> const& doc_root)
		: socket_(std::move(socket))
		, strand_(socket_.get_executor())
		, doc_root_(doc_root)
		, lambda_(*this)
	{
	}

	// Start the asynchronous operation
	void Run()
	{
		DoRead();
	}

	void DoRead()
	{
		// Make the request empty before reading,
		// otherwise the operation behavior is undefined.
		req_ = {};

		// Read a request
		boost::beast::http::async_read(socket_, buffer_, req_,
			boost::asio::bind_executor(
				strand_,
				std::bind(
					&session::OnRead,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2)));
	}

	void OnRead(
			boost::system::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// This means they closed the connection
		if (ec == boost::beast::http::error::end_of_stream)
			return do_close();

		if (ec)
			return fail(ec, "read");

		// Send the response
		HandleRequest(*doc_root_, std::move(req_), lambda_);
	}

	void
		on_write(
			boost::system::error_code ec,
			std::size_t bytes_transferred,
			bool close)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return fail(ec, "write");

		if (close)
		{
			// This means we should close the connection, usually because
			// the response indicated the "Connection: close" semantic.
			return do_close();
		}

		// We're done with the response so delete it
		res_ = nullptr;

		// Read another request
		DoRead();
	}

	void
		do_close()
	{
		// Send a TCP shutdown
		boost::system::error_code ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

		// At this point the connection is closed gracefully
	}
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket socket_;
	std::shared_ptr<std::string const> doc_root_;

public:
	listener(
		boost::asio::io_context& ioc,
		boost::asio::ip::tcp::endpoint endpoint,
		std::shared_ptr<std::string const> const& doc_root)
		: acceptor_(ioc)
		, socket_(ioc)
		, doc_root_(doc_root)
	{
		boost::system::error_code ec;

		// Open the acceptor
		acceptor_.open(endpoint.protocol(), ec);
		if (ec)
		{
			fail(ec, "open");
			return;
		}

		// Allow address reuse
		acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
		if (ec)
		{
			fail(ec, "set_option");
			return;
		}

		// Bind to the server address
		acceptor_.bind(endpoint, ec);
		if (ec)
		{
			fail(ec, "bind");
			return;
		}

		// Start listening for connections
		acceptor_.listen(
			boost::asio::socket_base::max_listen_connections, ec);
		if (ec)
		{
			fail(ec, "listen");
			return;
		}
	}

	// Start accepting incoming connections
	void
		Run()
	{
		if (!acceptor_.is_open())
			return;
		do_accept();
	}

	void
		do_accept()
	{
		acceptor_.async_accept(
			socket_,
			std::bind(
				&listener::on_accept,
				shared_from_this(),
				std::placeholders::_1));
	}

	void
		on_accept(boost::system::error_code ec)
	{
		if (ec)
		{
			fail(ec, "accept");
		}
		else
		{
			// Create the session and run it
			std::make_shared<session>(
				std::move(socket_),
				doc_root_)->Run();
		}

		// Accept another connection
		do_accept();
	}
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	// Check command line arguments.
	if (argc != 5)
	{
		std::cout <<
			"Usage: http-server-async <address> <port> <doc_root> <threads>\n" <<
			"Example:\n" <<
			"    http-server-async 0.0.0.0 8080 . 1\n";
		return EXIT_FAILURE;
	}
	auto const address = boost::asio::ip::make_address(argv[1]);
	auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
	auto const doc_root = std::make_shared<std::string>(argv[3]);
	auto const threads = std::max<int>(1, std::atoi(argv[4]));

	// The io_context is required for all I/O
	boost::asio::io_context l_IOContext{ threads };

	// Create and launch a listening port
	std::make_shared<listener>(
		l_IOContext,
		boost::asio::ip::tcp::endpoint{ address, port },
		doc_root)->Run();

	// Run the I/O service on the requested number of threads
	std::vector<std::thread> v;
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
	{
		v.emplace_back([&l_IOContext] { l_IOContext.run(); });
	}
	l_IOContext.run();

	return EXIT_SUCCESS;
}
