#pragma once

#include <boost/lexical_cast.hpp>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Client.h>
#include "olcPixelGameEngine.h"

#include "Player.h"


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

    
    uint8* ComposeMessage(NetworkLib::ClientMessageType type);

    NetworkLib::Client m_connection;

    Player m_player;

    long double m_totalTime = 0.0;

};