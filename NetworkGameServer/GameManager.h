#pragma once
#include <string>
#include <vector>
#include <map>
#include <common/tagOrDieCommon.h>
#include <common/Timer.h>
#include <NetworkLib/Server.h>

struct PlayerInput;

class GameManager
{
public:
    GameManager() {};
    ~GameManager() {};

    uint32 m_numPlayers = 0;
    // first->playerId, second->endpointId
    std::map<uint32, uint32_t> m_playerEndpointIds;
    // first->playerId, second->input
    std::map<uint32, PlayerInput> m_playerInputs;
    // first->tick, second->inputbuffer(playerId, input)
    std::multimap<uint32, std::pair<uint32, PlayerInput>> m_inputBuffer;
    // first->playerId, second->PlayerState
    std::map<uint32, PlayerState> m_playerStates;
    // first->playerId, second->latest received timestamp
    std::map<uint32, float32> m_clientTimeStamps;

    uint32 m_lastTickStatesSent = 0;
    uint32 m_currentTick = 0;

public:
    Timer m_timer;
    
    // Adds a new player to the game, return: first-> id, second->success
    std::pair<uint32, bool> AddPlayer(PlayerState state, uint32_t endpoint);
    // Remove player, returns false if error during removal
    bool RemovePlayer(uint32 id);
    void RemovePlayerByEndpoint(uint32 endpoint);

    // serialization functions return msg string
    std::string SerializeStatePackage(uint32 id);
    std::string SerializeAcceptPackage(PlayerState state, uint32 id);
    std::string SerializeRejectPackage();

    // tick local player and advance him in the simulation
    PlayerState Tick(const PlayerState& state, const PlayerInput& input);
    
    // Sends all players in game all states needed
    void SendStateToAllClients(NetworkLib::Server& server ); 

private:
};