#pragma once
#include <string>
#include <vector>
#include <map>
#include <tagOrDieCommon.h>

struct PlayerInput;

class GameManager
{
public:
    GameManager() {};
    ~GameManager() {};

    std::vector<PlayerInput> m_inputs;
    uint32 m_numPlayers = 0;
    // first->playerId, second->endpointId
    std::map<uint32, uint32_t> m_playerEndpointIds;
    // first->playerId, second->input
    std::map<uint32, PlayerInput> m_playerInputs;
    // first->playerId, second->PlayerState
    std::map<uint32, PlayerState> m_playerStates;

    // Adds a new player to the game, return: first-> id, second->success
    std::pair<uint32, bool> AddPlayer(PlayerState state, uint32_t endpoint);

    // returns false if error during removing
    bool RemovePlayer(uint32 id);
    void RemovePlayerByEndpoint(uint32 endpoint);

    std::string SerializeStatePackage(uint32 id);
    std::string SerializeAcceptPackage(PlayerState state, uint32 id);
    std::string SerializeRejectPackage();

    void UpdateState(const PlayerInput& input, int playerId, float32 dt);

    float32 m_currentTime = 0.0; // in seconds
    uint64  m_currentTicks = 0;
    uint64 GetCurrentTick();
    float32 TickToTime(uint64 tick); // TODO: move to common files
    uint64 TimeToTick(float32 time); // TODO: move to common files

private:
};