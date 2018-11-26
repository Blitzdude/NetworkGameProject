#pragma once
#include <string>
#include <utility>
#include <boost/asio.hpp>

namespace NetworkLib
{
	typedef std::pair<std::string, uint32_t> ClientMessage;

	class IServer 
	{
	public:
		virtual ~IServer() {};
		virtual bool HasMessages()  = 0;
		virtual void SendToClient(const std::string& message, uint32_t clientID)  = 0;
        virtual void SendToClient(const boost::asio::mutable_buffer& message, uint32_t clientID) = 0;

		virtual ClientMessage PopMessage()  = 0;
		virtual size_t GetClientCount()  = 0;
		virtual uint32_t GetClientIdByIndex(size_t)  = 0;
	};
}