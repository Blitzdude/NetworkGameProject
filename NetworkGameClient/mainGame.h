#pragma once

#include <boost/lexical_cast.hpp>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Client.h>
#include "olcPixelGameEngine.h"

#include "Player.h"

enum class GameState
{
    Disconnected,
    Joining,
    Joined,
    Running
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
    
    void Update(float fElapsedTime);
    void Draw();


    uint64 GetCurrentTick();
    float32 TickToTime(uint64 tick);
    uint64 TimeToTick(float32 time);
    
    NetworkLib::Client m_connection;

    Player m_player;
    std::map<uint32, PlayerState> m_otherPlayers;

    float32 m_currentTime = 0.0;
    uint64 m_currentTicks = 0;

    GameState m_gameState = GameState::Joining;

    bool isRunning = true;
};