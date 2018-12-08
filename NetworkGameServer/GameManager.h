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
    // first->playerId, second->PlayerState
    std::map<uint32, PlayerState> m_playerStates;

    std::string SerializeStatePackage(uint32 id);
    std::string SerializeAcceptPackage(PlayerState state, uint32 id);
    std::string SerializeRejectPackage();

    void UpdateState(const PlayerInput& input, int playerId, float32 dt);

    float64 m_totalTime = 0.0; // in seconds
    uint64 GetCurrentTick();
    float64 TickToTime(uint64 tick);
private:
};