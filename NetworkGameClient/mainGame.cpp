#include "mainGame.h"

bool MainGame::OnUserCreate() 
{
    // Called once at the start, so create things here
    // Create player - Values are used with reckless abandon, don't worry about it
    m_player.m_state.x = 100.0f;
    m_player.m_state.y = 100.0f;
    m_player.m_state.facing = 0.0f;
    m_player.m_state.speed = 30.0f;

    return true;
}

bool MainGame::OnUserUpdate(float fElapsedTime) 
{
    // called once per frame
    Clear(olc::BLACK);

    
    // update player
    if (GetKey(olc::D).bHeld) // turn left
    {
       m_player.m_state.facing += 1.0f * fElapsedTime;
    }
    else if (GetKey(olc::A).bHeld) // turn right
    {
        m_player.m_state.facing -= 1.0f * fElapsedTime;
    }
    if (GetKey(olc::W).bHeld) // forward
    {
        m_player.m_state.x += cosf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;
        m_player.m_state.y += sinf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;

    }
    else if (GetKey(olc::S).bHeld) // back
    {
        m_player.m_state.x -= cosf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;
        m_player.m_state.y -= sinf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;
    }

    float l_playerX = m_player.m_state.x;
    float l_playerY = m_player.m_state.y;
    float l_facing = m_player.m_state.facing;

    // draw player
    DrawCircle(l_playerX, l_playerY, 10);

    DrawLine(m_player.m_state.x, m_player.m_state.y, 
             m_player.m_state.x + cosf(l_facing) * 10.0f,
             m_player.m_state.y + sinf(l_facing) * 10.0f, olc::MAGENTA);

    return true;
}