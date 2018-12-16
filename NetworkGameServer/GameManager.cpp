#include "GameManager.h"
#include <sstream>
#include <NetworkLib/Constants.h>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>


std::pair<uint32, bool> GameManager::AddPlayer(PlayerState state, uint32_t endpoint)
{
    bool l_success = true;
    uint32 l_id = std::numeric_limits<uint32>::max(); // init to out of scope

    // first check if the is a maximum number of players already
    if (m_numPlayers >= TOD_MAX_CLIENTS)
    {
        l_success = false;
    }
    else
    {
        for (unsigned int i = 0; i < m_playerEndpointIds.size() + 1; ++i)
        {
            if (m_playerEndpointIds.find(i) == m_playerEndpointIds.end())
            {
                // id found,
                // NOTE: If no id is found before .size(), then the last
                // .size() + 1 should be the id;
                l_id = i;
                break;
            }
        }
        assert(l_id < m_numPlayers + 1);
        m_numPlayers++;
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

std::string GameManager::SerializeStatePackage(uint32 id)
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);

    l_archive << NetworkLib::ServerMessageType::State << m_numPlayers
    << m_currentTick << m_clientTimeStamps[id];

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
        l_archive << l_localId << l_localState;


        // then add the rest
        for (auto itr = m_playerStates.begin(); itr != m_playerStates.end(); itr++)
        {
            if (itr->first != id)
            {
                uint32 l_otherId = itr->first;
                PlayerState l_otherState = itr->second;
                l_archive << l_otherId << l_otherState;
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

    l_archive << (uint8)NetworkLib::ServerMessageType::Accept;
    l_archive << id;                                        
    return oss.str();
}

std::string GameManager::SerializeRejectPackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    l_archive << (uint8)NetworkLib::ServerMessageType::Reject;

    return oss.str();
}
// TODO: same as in Player.h/cpp. Move player to common code base
PlayerState GameManager::Tick(const PlayerState & state, const PlayerInput & input)
{
    PlayerState l_ret = state;
    if (input.up)
    {
        l_ret.x += cosf(l_ret.facing) * c_speed* c_seconds_per_tick;
        l_ret.y += sinf(l_ret.facing) * c_speed * c_seconds_per_tick;
    }
    if (input.down)
    {
        l_ret.x -= cosf(l_ret.facing) * c_speed * c_seconds_per_tick;
        l_ret.y -= sinf(l_ret.facing) * c_speed * c_seconds_per_tick;
    }
    if (input.left)
    {
        l_ret.facing += c_turn_speed * c_seconds_per_tick;
    }
    if (input.right)
    {
        l_ret.facing -= c_turn_speed * c_seconds_per_tick;
    }

    return l_ret;
}

void GameManager::SendStateToAllClients(NetworkLib::Server& server)
{
    for (auto itr : m_playerEndpointIds)
    {
        server.SendToClient(SerializeStatePackage(itr.first), itr.second);
    }
}

