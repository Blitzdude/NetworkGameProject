#pragma once
#include <string>

namespace NetworkLib
{
	class IClient 
	{
	public:
		virtual ~IClient() {};
		virtual bool HasMessages()  = 0;
		virtual void Send(const std::string& message)  = 0;
		virtual std::string PopMessage()  = 0;
	};
}