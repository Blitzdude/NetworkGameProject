#include "mainGame.h"

bool MainGame::OnUserCreate() 
{
    // Called once at the start, so create things here
    // Create player - Values are used with reckless abandon, don't worry about it
    m_player.m_state.x = 100.0f;
    m_player.m_state.y = 100.0f;
    m_player.m_state.facing = 0.0f;
    m_player.m_state.speed = 30.0f;

    m_connection.Send(ComposeMessage(NetworkLib::ClientMessageType::Join));

    return true;
}

bool MainGame::OnUserUpdate(float fElapsedTime) 
{
    // called once per frame
    Clear(olc::BLACK);

    Update(fElapsedTime);
    if (m_player.HasInput())
    {
        m_connection.Send(ComposeMessage(NetworkLib::ClientMessageType::Input));
    }
    Draw();

    return true;
}

bool MainGame::OnUserDestroy()
{
    m_connection.Send(ComposeMessage(NetworkLib::ClientMessageType::Leave));

    return true;
}

void MainGame::Update(float fElapsedTime)
{

    // update player
    if (GetKey(olc::D).bHeld) // turn left
    {
        m_player.m_input.right = false;
        m_player.m_input.left = true;
        m_player.m_state.facing += 1.0f * fElapsedTime;
    }
    else if (GetKey(olc::A).bHeld) // turn right
    {
        m_player.m_input.right = true;
        m_player.m_input.left = false;
        m_player.m_state.facing -= 1.0f * fElapsedTime;
    }
    if (GetKey(olc::W).bHeld) // forward
    {
        m_player.m_input.up = true;
        m_player.m_input.down = false;

        m_player.m_state.x += cosf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;
        m_player.m_state.y += sinf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;

    }
    else if (GetKey(olc::S).bHeld) // back
    {
        m_player.m_input.up = false;
        m_player.m_input.down = true;

        m_player.m_state.x -= cosf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;
        m_player.m_state.y -= sinf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;
    }

    

}

void MainGame::Draw()
{

    float l_playerX = m_player.m_state.x;
    float l_playerY = m_player.m_state.y;
    float l_facing = m_player.m_state.facing;

    // draw player
    DrawCircle((int32_t)l_playerX, (int32_t)l_playerY, 10);

    DrawLine(m_player.m_state.x, m_player.m_state.y,
        m_player.m_state.x + cosf(l_facing) * 10.0f,
        m_player.m_state.y + sinf(l_facing) * 10.0f, olc::MAGENTA);

}

/*
    Client msg buffer:
    uint8: type | uint16: id | uint32: data
*/
std::string MainGame::ComposeMessage(NetworkLib::ClientMessageType type)
{  
    std::string l_msg;

    switch (type)
    {
    case NetworkLib::ClientMessageType::Join:
        l_msg.append("Join");
        break;
    case NetworkLib::ClientMessageType::Leave:
        l_msg.append("Leave");
        break;
    case NetworkLib::ClientMessageType::Input:
        l_msg.append("Input");
        break;

    default:
        break;
    }

    return l_msg;
}



void MainGame::SendMessageToServer(std::string p_msg)
{
    m_connection.Send(p_msg);
}

