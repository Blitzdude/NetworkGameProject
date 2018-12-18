#pragma once

#include <boost/lexical_cast.hpp>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Client.h>
#include "olcPixelGameEngine.h"
#include <common/Timer.h>

#include "Player.h"

enum class GameState
{
    Disconnected,
    Joining,
    Joined,
};

class MainGame : public olc::PixelGameEngine
{
public:
    MainGame()
        : m_connection("127.0.0.1", 8080)
    {
        sAppName = "NetworkGameProgram";
    }
    
    MainGame(int argc, char* argv[])
        : m_connection(argv[1], boost::lexical_cast<int>(argv[2]))
    {
        sAppName = "TagOrDie";
    }
    
public:
    bool OnUserCreate() override;
    bool OnUserUpdate(float fElapsedTime) override;
    bool OnUserDestroy() override;
private:
    
    void Update();
    void Draw();
    PlayerState LerpPlayerState(const PlayerState& a, const PlayerState& b, float32 t0, float32 t1, float32 t);
    // used to lerp other players ahead
    uint32 m_deltaLerp = 0;
    std::string SerializeJoinPackage();
    std::string SerializeLeavePackage();
    
    NetworkLib::Client m_connection;

    Timer m_timer;

    Player m_player;
    std::map<uint32, PlayerState> m_otherPlayersCurrentPosition;
    // other player first->slot, second->list(tick, state)
    std::map<uint32, std::pair<uint32,PlayerState>> m_otherPlayersLastKnownState;

    PlayerState m_localPlayerServerState;

    //float32 m_currentTime = 0.0f;
    uint32 m_currentTicks = 0;
    uint32 m_targetTickNumber = 0;

    GameState m_gameState = GameState::Joining;

    bool isRunning = true;
};