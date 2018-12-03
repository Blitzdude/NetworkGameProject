#include <iostream>
#include <list>
#include <NetworkLib/Server.h>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>
#include <common/tagOrDieCommon.h>
#include <NetworkLib/Statistics.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

/*
* TODO:
* Multiple player states
* Time synchronisation

*/

class GameManager
{
public:
    GameManager(){};
    ~GameManager(){};

    std::vector<PlayerInput> m_inputs;
    uint32 m_numPlayer = 0;
    std::map<uint32, PlayerState> m_playerStates;

    std::string SerializeStatePackage();

    void UpdateState(const PlayerInput& input, int playerId);
    
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
            auto l_msg = l_server.PopMessage();
            Log::Debug(l_msg.first, "size; ", l_msg.second);

            std::istringstream iss(l_msg.first);    
            boost::archive::text_iarchive iar(iss);

            uint8 id;
            PlayerInput input;
            long double time;
            PlayerState l_np;

            NetworkLib::ClientMessageType type;
            iar >> type;
            switch (type)
            {
            case NetworkLib::ClientMessageType::Join:
                Log::Debug("Player Joined");
                // add player to list of players
                l_np = { 30.0f,30.0f,0.0f, 10.0f };
                l_gm.m_playerStates.emplace(l_gm.m_numPlayer++, l_np);
                l_gm.m_numPlayer++;
                // send player their starting position
                l_server.SendToAll(l_gm.SerializeStatePackage());

                break;
            case NetworkLib::ClientMessageType::Leave:
                // Remove the player from player states
                break;
            case NetworkLib::ClientMessageType::Input:
                Log::Debug("Player input get!");
                iar >> id;
                iar >> input;
                iar >> time;
                Log::Debug("Input:", input.up, input.down, input.left, input.right);

                // Send server package to client
                // number of clients
                l_gm.UpdateState(input, id);
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

std::string GameManager::SerializeStatePackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    // msgType : numPlayrs : Playerstate x numplayers
    l_archive << NetworkLib::ServerMessageType::State << m_numPlayer;
    for (auto itr = m_playerStates.begin(); itr != m_playerStates.end(); itr++)
    {
       l_archive << itr;
    }

    return oss.str();
}

void GameManager::UpdateState(const PlayerInput& input, int playerId)
{
    const float fElapsedTime = 0.01f;

    // update player
    if (input.left) // turn left
    {
        m_playerStates[playerId].facing += 1.0f * fElapsedTime;
    }
    if (input.right) // turn right
    {
        m_playerStates[playerId].facing -= 1.0f * fElapsedTime;
    }
    if (input.up) // forward
    {

        m_playerStates[playerId].x += cosf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * fElapsedTime;
        m_playerStates[playerId].y += sinf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * fElapsedTime;

    }
    if (input.down) // back
    {
        m_playerStates[playerId].x -= cosf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * fElapsedTime;
        m_playerStates[playerId].y -= sinf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * fElapsedTime;
    }
}
