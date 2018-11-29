#include <iostream>
#include <list>
#include <NetworkLib/Server.h>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>
#include <common/tagOrDieCommon.h>
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
    std::vector<PlayerInput> m_inputs;
    uint32 m_numPlayer = 0;
    //std::map<uint32, PlayerState> m_states;
    PlayerState m_playerState = {0.0f, 0.0f, 0.0f, 10.0f};

    std::string SerializeStatePackage();

    void UpdateState(const PlayerInput& input);
    
private:
    
};

int main(int argc, char* argv[])
{
    NetworkLib::Server l_server(8080);
    GameManager l_gm;

    
    bool isRunning = true;
    while (isRunning)
    {
        while (l_server.HasMessages())
        {
            //TODO istringstream -> archive -> struct?
            auto l_msg = l_server.PopMessage();
            Log::Debug(l_msg.first, "size; ", l_msg.second);

            std::istringstream iss(l_msg.first);    
            boost::archive::text_iarchive iar(iss);

            uint8 id;
            PlayerInput input;
            long double time;
            PlayerState l_np = {0.0f,0.0f,0.0f,0.0f};


            NetworkLib::ClientMessageType type;
            iar >> type;
            switch (type)
            {
            case NetworkLib::ClientMessageType::Join:
                Log::Debug("Player Joined");
                /*
                l_gm.m_states.insert(
                    std::pair<uint32, PlayerState>(l_gm.m_numPlayer,l_np));
                */
                l_np = { 0.0f,0.0f,0.0f,0.0f };
                l_gm.m_numPlayer++;
                break;
            case NetworkLib::ClientMessageType::Leave:
                break;
            case NetworkLib::ClientMessageType::Input:
                Log::Debug("Player input get!");
                iar >> id;
                iar >> input;
                iar >> time;
                Log::Debug("Input:", input.up, input.down, input.left, input.right);

                // Send server package to client
                // number of clients
                l_gm.UpdateState(input);
                l_server.SendToAll(l_gm.SerializeStatePackage());

                break;
            default:
                break;
            }

            // Average byte size of message
            Log::Info("Average bytes: "
                , l_server.GetStatistics().GetReceivedBytes()
                , "//", l_server.GetStatistics().GetReceivedMessages()
                , "=", l_server.GetStatistics().GetReceivedBytes()
                / l_server.GetStatistics().GetReceivedMessages());

        }
    }

    std::cin.get();
    return 0;
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

std::string GameManager::SerializeStatePackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    // msgType : numPlayrs : Playerstate x numplayers
    l_archive << NetworkLib::ServerMessageType::State << m_numPlayer
        << m_playerState;
    

    return oss.str();
}

void GameManager::UpdateState(const PlayerInput& input)
{
    const float fElapsedTime = 0.1f;

    // update player
    if (input.left) // turn left
    {
        m_playerState.facing += 1.0f * fElapsedTime;
    }
    if (input.right) // turn right
    {
        m_playerState.facing -= 1.0f * fElapsedTime;
    }
    if (input.up) // forward
    {

        m_playerState.x += cosf(m_playerState.facing) * m_playerState.speed * fElapsedTime;
        m_playerState.y += sinf(m_playerState.facing) * m_playerState.speed * fElapsedTime;

    }
    if (input.down) // back
    {
        m_playerState.x -= cosf(m_playerState.facing) * m_playerState.speed * fElapsedTime;
        m_playerState.y -= sinf(m_playerState.facing) * m_playerState.speed * fElapsedTime;
    }
}
