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
    // first->playerId, second->inputbuffer(tick, input)
    std::multimap<uint64, std::pair<uint32, PlayerInput>> m_inputBuffer;
    // first->playerId, second->PlayerState
    std::map<uint32, PlayerState> m_playerStates;

    uint64 m_lastTickStatesSent = 0;

public:
    // Adds a new player to the game, return: first-> id, second->success
    std::pair<uint32, bool> AddPlayer(PlayerState state, uint32_t endpoint);
    // returns false if error during removal
    bool RemovePlayer(uint32 id);
    void RemovePlayerByEndpoint(uint32 endpoint);

    // serialization functions return the msg string
    std::string SerializeStatePackage(uint32 id);
    std::string SerializeAcceptPackage(PlayerState state, uint32 id);
    std::string SerializeRejectPackage();

    PlayerState Tick(const PlayerState& state, const PlayerInput& input);
    void UpdateState(const PlayerInput& input, int playerId, float32 dt);
    void SendStateToAllClients(NetworkLib::Server& server ); // TODO: const correctness?


    Timer m_timer;

private:
};