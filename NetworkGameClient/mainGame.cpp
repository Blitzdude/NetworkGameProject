#include "mainGame.h"
#include <boost/asio.hpp>
#include <NetworkLib/Constants.h>
#include <NetworkLib/Log.h>
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
    m_player.m_input.up = false;
    m_player.m_input.down = false;
    m_player.m_input.left = false;
    m_player.m_input.right = false;


    // update player
    if (GetKey(olc::D).bHeld) // turn left
    {
        m_player.m_input.left = true;
        m_player.m_state.facing += 1.0f * fElapsedTime;
    }
    else if (GetKey(olc::A).bHeld) // turn right
    {
        m_player.m_input.right = true;
        m_player.m_state.facing -= 1.0f * fElapsedTime;
    }
    if (GetKey(olc::W).bHeld) // forward
    {
        m_player.m_input.up = true;
        
        m_player.m_state.x += cosf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;
        m_player.m_state.y += sinf(m_player.m_state.facing) * m_player.m_state.speed * fElapsedTime;

    }
    else if (GetKey(olc::S).bHeld) // back
    {
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
    uint8: type | uint8: id | uint32: data
*/
boost::asio::mutable_buffer MainGame::ComposeMessage(NetworkLib::ClientMessageType type)
{  
    boost::asio::mutable_buffer ret;
    uint8 l_clientPackage[NetworkBufferSize];

    switch (type)
    {
    case NetworkLib::ClientMessageType::Join:
        
        
        l_clientPackage[0] = (uint8)NetworkLib::ClientMessageType::Join;
        ret = boost::asio::buffer(l_clientPackage);

        break;
    case NetworkLib::ClientMessageType::Leave:

        l_clientPackage[0] = (uint8)NetworkLib::ClientMessageType::Leave;
        ret = boost::asio::buffer(l_clientPackage);
        break;
    case NetworkLib::ClientMessageType::Input:

        m_player.WriteInputPacket(l_clientPackage);
        ret = boost::asio::buffer(l_clientPackage);
        
        break;
    default:
        break;
    }
    Log::Debug(ret.data());
    return ret;
}

void MainGame::SendMessageToServer(std::string p_msg)
{
    m_connection.Send(p_msg);
}

