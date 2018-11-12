#pragma once
#include "olcPixelGameEngine.h"
#include "Player.h"


class MainGame : public olc::PixelGameEngine
{
public:
    MainGame()
    {
        sAppName = "Example";
    }

public:
    bool OnUserCreate() override;

    bool OnUserUpdate(float fElapsedTime) override;
private:
    Player m_player;

};