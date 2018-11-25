#pragma once

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

public:
    bool OnUserCreate() override;
    bool OnUserUpdate(float fElapsedTime) override;
    bool OnUserDestroy() override;
private:

    void Update(float fElapsedTime);
    void Draw();

    std::string ComposeMessage(NetworkLib::ClientMessageType type);
    void SendMessageToServer(std::string p_msg);

    NetworkLib::Client m_connection;

    Player m_player;

};