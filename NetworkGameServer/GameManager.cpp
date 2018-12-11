#include "GameManager.h"
#include <sstream>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>

// Add new player, returns true if succesful, otherwise false
std::pair<uint32, bool> GameManager::AddPlayer(PlayerState state, uint32_t endpoint)
{
    bool l_success = true;
    uint32 l_id = (0U - 1U); // init to out of scope

    // first check if the is a maximum number of players already
    if (m_numPlayers >= TOD_MAX_CLIENTS)
    {
        l_success = false;
    }
    else
    {
        l_id = m_numPlayers++;
    }
    
    if (l_success) // TODO: Try-catch struct for == .end() results?
    {
        if (m_playerStates.find(l_id) != m_playerStates.end() && l_success)
        {
            // if insertion wasn't succesful
            Log::Debug("PlayerState already exists", l_id);
            l_success = false;
        }
        else
        {
            m_playerStates.emplace(l_id, state);
        }

        if (m_playerInputs.find(l_id) != m_playerInputs.end() && l_success)
        {
            // if insertion wasn't succesful
            Log::Debug("PlayerInput already exists", l_id);
            l_success = false;
        }
        else
        {
            m_playerInputs.emplace(l_id, PlayerInput{false, false, false, false});
        }

        if (m_playerEndpointIds.find(l_id) != m_playerEndpointIds.end() && l_success)
        {
            // if insertion wasn't succesful
            Log::Debug("PlayerEndpoint already exists", l_id);
            l_success = false;
        }
        else
        {
            m_playerEndpointIds.emplace(l_id, endpoint);
        }
    }

    // if any of the above fail, retuns false
    return std::make_pair(l_id , l_success);
}

bool GameManager::RemovePlayer(uint32 id)
{
    bool success = true;
    // first check if the is a maximum number of players already
    assert(m_numPlayers > 0); // Make sure the number of players is NOT 0
    
    // map.erase(K) returns the amount of elements removed
    // in this case: 1 if key was found

    if (!m_playerInputs.erase(id))
    {
        Log::Debug("No Playerinput was erased ", id);
        success = false;
    }

    if (!m_playerStates.erase(id))
    {
        Log::Debug("No PlayerState was erased ", id);
        success = false;
    }

    if (!m_playerEndpointIds.erase(id))
    {
        Log::Debug("No Endpoint was erased ", id);
        success = false;
    }
    if (success)
    {
        m_numPlayers--;
    }

    return success;
}

void GameManager::RemovePlayerByEndpoint(uint32 endpoint)
{
    // get the id from endpointId-map
    uint32 l_id = std::numeric_limits<uint32>::max(); // set to max value for debugging
    for (auto itr : m_playerEndpointIds)
    {
        if (endpoint == itr.second)
        {
            l_id = itr.first; // we have found the id
        }
    }
    // if l_id is the maximum value, it will not find it from anywere.
    RemovePlayer(l_id); 
}

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
        // Log::Debug("Local", l_localId, l_localState.x, l_localState.y);
        l_archive << l_localId << l_localState;


        // then add the rest
        for (auto itr = m_playerStates.begin(); itr != m_playerStates.end(); itr++)
        {
            if (itr->first != id)
            {
                uint32 l_otherId = itr->first;
                PlayerState l_otherState = itr->second;
                l_archive << l_otherId << l_otherState;
                // Log::Debug("Other: ", l_otherId, l_otherState.x, l_otherState.y ,++l_count);
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

    l_archive << NetworkLib::ServerMessageType::Accept;     // MsgType 
    l_archive << id;                                        // ID
    l_archive << GetCurrentTick();                          // Tick
    l_archive << state;                                     // State


    return oss.str();
}

std::string GameManager::SerializeRejectPackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    l_archive << (uint8)NetworkLib::ServerMessageType::Reject;

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
    return static_cast<uint64>(time * ticks_per_second);
}
