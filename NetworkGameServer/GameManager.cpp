#include "GameManager.h"
#include <sstream>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>

/* State packackage structure:
| msgType | tickNum | Client timestamp | LocalPlayerState | Other PlayerState | Other PlayerState | ... */
std::string GameManager::SerializeStatePackage(uint32 id)
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);

    //| MsgType | NumPlayers | ServerTick | LPTimeStamp |
    l_archive << NetworkLib::ServerMessageType::State << m_numPlayers;
    l_archive << GetCurrentTick() << m_currentTime;

    // Add the local player state first
    try
    {
        auto l_localPlayer = m_playerStates.find(id);

        if (l_localPlayer == m_playerStates.end())
        {
            Log::Error("No player of id: ", id, " found");
        }

        uint32 l_localId = l_localPlayer->first;
        PlayerState l_localState = l_localPlayer->second;
        Log::Debug("Local", l_localId, l_localState.x, l_localState.y);
        l_archive << l_localId << l_localState;


        // then add the rest
        for (auto itr = m_playerStates.begin(); itr != m_playerStates.end(); itr++)
        {
            if (itr->first != id)
            {
                uint32 l_otherId = itr->first;
                PlayerState l_otherState = itr->second;
                l_archive << l_otherId << l_otherState;
                Log::Debug("Other: ", l_otherId, l_otherState.x, l_otherState.y );
            }
        }
    }
    catch (const std::out_of_range eoor)
    {
        Log::Debug("Exception: out of range", eoor.what());
    }
    catch (const std::exception& e)
    {
        Log::Debug("Exception: ", e.what());
    }

    return oss.str();
}

std::string GameManager::SerializeAcceptPackage(PlayerState state, uint32 id)
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);

    l_archive << NetworkLib::ServerMessageType::Accept;      // MsgType 
    l_archive << id;                                        // ID
    l_archive << GetCurrentTick();                           // Tick
    l_archive << state;                                     // State


    return oss.str();
}

std::string GameManager::SerializeRejectPackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    l_archive << NetworkLib::ServerMessageType::Reject;

    return oss.str();
}

void GameManager::UpdateState(const PlayerInput& input, int playerId, float32 dt)
{
    try
    {
        auto &l_player = m_playerStates.at(playerId);

        // update player
        if (input.left) // turn left
        {
            l_player.facing += 1.0f * dt;
        }
        if (input.right) // turn right
        {
            l_player.facing -= 1.0f * dt;
        }
        if (input.up) // forward
        {
            l_player.x += cosf(l_player.facing) * 100.0f * dt;
            l_player.y += sinf(l_player.facing) * 100.0f * dt;

        }
        if (input.down) // back
        {
            l_player.x -= cosf(l_player.facing) * 100.0f * dt;
            l_player.y -= sinf(l_player.facing) * 100.0f * dt;
        }
     }
    catch (const std::out_of_range& eoor)
    {
        Log::Debug("Exception, Out of range: No player found with id: "
                    ,playerId , eoor.what());
    }
    catch (const std::exception& e)
    {
        Log::Debug("Exception", e.what());
    }

}

uint64 GameManager::GetCurrentTick()
{

    return static_cast<uint64>(m_currentTime * ticks_per_second);
}

float32 GameManager::TickToTime(uint64 tick)
{
    return tick * seconds_per_tick;
}

uint64 GameManager::TimeToTick(float32 time)
{
    return time * ticks_per_second;
}
