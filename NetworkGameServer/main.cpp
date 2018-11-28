#include <iostream>
#include <list>
#include <NetworkLib/Server.h>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>
#include <NetworkLib/Statistics.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

boost::asio::mutable_buffer ComposeMessage(NetworkLib::ServerMessageType type);

class GameManager
{
public:
    GameManager(){};
    ~GameManager(){};

    std::list<uint32_t> m_clientIds;


private:
    
};

int main(int argc, char* argv[])
{
    NetworkLib::Server l_server(8080);

    boost:
    GameManager l_gm;

    bool isRunning = true;
    while (isRunning)
    {
        while (l_server.HasMessages())
        {
            //TODO istringstream -> archive -> struct?
            auto l_msg = l_server.PopMessage();

            NetworkLib::ClientMessageType l_type = 
                static_cast<NetworkLib::ClientMessageType>(l_msg.first[0]);

            boost::asio::mutable_buffer l_toClient;
            Log::Debug(l_msg.first);

            switch (l_type)
            {
            case NetworkLib::ClientMessageType::Join:
            Log::Info("Join message received");
                if (l_gm.m_clientIds.size() < TOD_MAX_CLIENTS)
                {
                    l_gm.m_clientIds.emplace_back(l_msg.second);
                    l_toClient = 
                        ComposeMessage(NetworkLib::ServerMessageType::Accept);
                }
                else
                {
                    l_toClient = 
                        ComposeMessage(NetworkLib::ServerMessageType::Reject);
                }
                l_server.SendToClient(l_toClient, l_msg.second);

                break;
            case NetworkLib::ClientMessageType::Leave:
                Log::Info("Leave message received");
                // Remove client from clients list
                l_gm.m_clientIds.remove_if([&l_msg](uint32_t n){return l_msg.second;});
                break;
            case NetworkLib::ClientMessageType::Input:
                Log::Info("Input message received", (uint32)l_msg.first[1]);
                

                l_toClient = 
                    ComposeMessage(NetworkLib::ServerMessageType::State);
                l_server.SendToAll(l_toClient);
                break;
            default:
                Log::Debug("Package doesn't correspond to any type");
                Log::Debug(l_server.GetStatistics().GetReceivedMessages() );
                break;
            }
        }
    }

}

boost::asio::mutable_buffer ComposeMessage(NetworkLib::ServerMessageType type)
{
    boost::asio::mutable_buffer ret;
    uint8 msg[NetworkBufferSize];
    switch (type)
    {
    case NetworkLib::ServerMessageType::Accept:
        // WriteAcceptPackage
        msg[0] = (uint8)NetworkLib::ServerMessageType::Accept;
        msg[sizeof(uint8)] = (uint8)1;
        ret = boost::asio::buffer(msg);
        break;
    case NetworkLib::ServerMessageType::Reject:
        // WriteRejectPackage
        msg[0] = (uint8)NetworkLib::ServerMessageType::Reject;
        ret = boost::asio::buffer(msg);
        break;
    case NetworkLib::ServerMessageType::State:
        // WriteStatePackage
        msg[0] = (uint8)NetworkLib::ServerMessageType::State;
        ret = boost::asio::buffer(msg);
        break;
    default:
        break;
    }
    return ret;
}
