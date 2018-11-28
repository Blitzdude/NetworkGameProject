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

    auto msg = ComposeMessage(NetworkLib::ClientMessageType::Join);
    auto buf = boost::asio::buffer(msg, sizeof(128));
    m_connection.Send(buf);

    return true;
}

bool MainGame::OnUserUpdate(float fElapsedTime) 
{
    m_totalTime += fElapsedTime;
    // called once per frame
    std::ostringstream ss;
    boost::archive::text_oarchive l_archive(ss);

    Clear(olc::BLACK);

    if (m_connection.HasMessages())
        m_connection.PopMessage();

    Update(fElapsedTime);
    if (m_player.HasInput())
    {
        l_archive << m_player;
        m_connection.Send(ss.str());
         


        /*
        auto msg = ComposeMessage(NetworkLib::ClientMessageType::Input);
        auto buf = boost::asio::buffer(msg, 128);
        m_connection.Send(buf);
        */
    }
    Draw();

    return true;
}

bool MainGame::OnUserDestroy()
{
    auto msg = ComposeMessage(NetworkLib::ClientMessageType::Leave);
    auto buf = boost::asio::buffer(msg, 128);
    m_connection.Send(buf);

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
    if (GetKey(olc::A).bHeld) // turn right
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
    if (GetKey(olc::S).bHeld) // back
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
uint8* MainGame::ComposeMessage(NetworkLib::ClientMessageType type)
{  
    //std::string ret;
    //ret.resize(256);
    uint8 ret[128];
    switch (type)
    {
    case NetworkLib::ClientMessageType::Join:
        ret[0] = static_cast<uint8>(NetworkLib::ClientMessageType::Join);
        break;
    case NetworkLib::ClientMessageType::Leave:
        ret[0] = static_cast<uint8>(NetworkLib::ClientMessageType::Leave);
        break;
    case NetworkLib::ClientMessageType::Input:
        ret[0] = static_cast<uint8>(NetworkLib::ClientMessageType::Input);
        /*
        packed_input =   m_player.m_input.up ? 1U : 0 |
                         m_player.m_input.down ? (1U << 1) : (0U << 1) |
                         m_player.m_input.left ? (1U << 2) : (0U << 2) |
                         m_player.m_input.right ? (1U << 3): (0U << 3) ;

        */
        Log::Debug(ret);
        break;
    default:
        break;
    }
    
    return ret;
}
