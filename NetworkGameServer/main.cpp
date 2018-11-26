#include <iostream>
#include <list>
#include <NetworkLib/Server.h>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>

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
    GameManager l_gm;
    bool isRunning = true;
    while (isRunning)
    {
        if (l_server.GetClientCount() > 0)
        {
            while (l_server.HasMessages())
            {
                auto l_msg = l_server.PopMessage();
                
                auto l_type = 
                    (NetworkLib::ClientMessageType)l_msg.first[0];
                Log::Debug(l_type);
                boost::asio::mutable_buffer l_toClient;
                switch (l_type)
                {
                case NetworkLib::ClientMessageType::Join:
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
                    // Remove client from clients list
                    l_gm.m_clientIds.remove_if([&l_msg](uint32_t n){return l_msg.second;});
                    break;
                case NetworkLib::ClientMessageType::Input:
                    l_toClient = 
                        ComposeMessage(NetworkLib::ServerMessageType::State);
                    l_server.SendToAll(l_toClient);
                    break;
                default:
                    break;
                }



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
