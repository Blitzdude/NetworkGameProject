#include <iostream>
#include <list>
#include <chrono>
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

    void UpdateState(const PlayerInput& input, int playerId, float32 dt);

    float64 m_totalTime = 0.0; // in seconds
    
    
    uint64 GetCurrentTick();
private:
};

int main(int argc, char* argv[])
{
    NetworkLib::Server l_server(8080);
    GameManager l_gm;

    auto l_tp1 = std::chrono::system_clock::now();
    auto l_tp2 = std::chrono::system_clock::now();

    
    bool isRunning = true;
    while (isRunning)
    {
        // Handle timing
        l_tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = l_tp2 - l_tp1;
        l_tp1 = l_tp2;

        float32 l_fElapsedTime = elapsedTime.count();
        l_gm.m_totalTime += l_fElapsedTime;
        
        Log::Debug(l_gm.GetCurrentTick(), l_gm.m_totalTime);

        while (l_server.HasMessages())
        {
            auto l_msg = l_server.PopMessage();
            Log::Debug(l_msg.first, "size; ", l_msg.second);

            std::istringstream iss(l_msg.first);    
            boost::archive::text_iarchive iar(iss);

            uint8 l_id;
            PlayerInput l_input;
            uint64 l_time;
            PlayerState l_np;

            NetworkLib::ClientMessageType l_type;
            iar >> l_type;
            switch (l_type)
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
                iar >> l_id;
                iar >> l_input;
                iar >> l_time;
                Log::Debug("Input:",l_id, l_input.up, l_input.down, l_input.left, l_input.right);

                // update client position on server
                l_gm.UpdateState(l_input, l_id, l_fElapsedTime);
                // Send server package to clients
                l_server.SendToAll(l_gm.SerializeStatePackage());

                break;
            default:
                break;
            }
        }

        // Average byte size of message
        if (l_server.GetStatistics().GetReceivedMessages() != 0) // stop division by zero error
        {
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
/* State packackage structure:
    | msgType | tickNum | Client timestamp | LocalPlayerState | Other PlayerState | Other PlayerState | ... */
std::string GameManager::SerializeStatePackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);

    // msgType : numPlayers : (id ::Playerstate)  x numplayers
    l_archive << NetworkLib::ServerMessageType::State << m_numPlayer;

    for (auto itr = m_playerStates.begin(); itr != m_playerStates.end(); itr++)
    {
       l_archive << itr->first << itr->second;
    }

    return oss.str();
}

void GameManager::UpdateState(const PlayerInput& input, int playerId, float32 dt)
{
    
    

    // update player
    if (input.left) // turn left
    {
        m_playerStates[playerId].facing += 1.0f * dt;
    }
    if (input.right) // turn right
    {
        m_playerStates[playerId].facing -= 1.0f * dt;
    }
    if (input.up) // forward
    {

        m_playerStates[playerId].x += cosf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * dt;
        m_playerStates[playerId].y += sinf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * dt;

    }
    if (input.down) // back
    {
        m_playerStates[playerId].x -= cosf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * dt;
        m_playerStates[playerId].y -= sinf(m_playerStates[playerId].facing) * m_playerStates[playerId].speed * dt;
    }
}

uint64 GameManager::GetCurrentTick()
{
    
    return  static_cast<uint64>(m_totalTime * ticks_per_second);
}
