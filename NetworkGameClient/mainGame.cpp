#include "mainGame.h"
#include <boost/asio.hpp>
#include <NetworkLib/Constants.h>
#include <NetworkLib/Log.h>

/*
* Tag Or Die - client
* Client states:
* - Joining
* - Joined
* - Game beginning
* - Game running
* - Game ended
* - Disconnected
* 
*/
bool MainGame::OnUserCreate() 
{
    // Create player - Values are used with reckless abandon, don't worry about it
    // m_player.m_state.x = 100.0f;
    // m_player.m_state.y = 100.0f;
    // m_player.m_state.facing = 0.0f;
    // m_player.m_state.speed = 30.0f;

    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    // Send a Join Request to server
    l_archive << NetworkLib::ClientMessageType::Join;

    m_connection.Send(oss.str());

    return true;
}

bool MainGame::OnUserUpdate(float fElapsedTime) 
{
    m_totalTime += fElapsedTime;
    m_player.m_playerTime = m_totalTime;
    // called once per frame
    std::ostringstream oss;
    boost::archive::text_oarchive l_oar(oss);

    Clear(olc::BLACK);

    if (m_connection.HasMessages())
        m_connection.PopMessage();

     Update(fElapsedTime);
    if (m_player.HasInput())
    {
        m_connection.Send(m_player.SerializeInput());
    }
    while (m_connection.HasMessages())
    {
        auto l_msg = m_connection.PopMessage();

        std::istringstream iss(l_msg);
        boost::archive::text_iarchive iar(iss);
        // for network pacakges
        uint32 numPlayers;
        PlayerState l_state;
        std::vector<PlayerState> l_playerStates;

        NetworkLib::ServerMessageType type;
        iar >> type;
        switch (type)
        {
        case NetworkLib::ServerMessageType::Accept:
            Log::Debug("Joining game");
            iar >> l_state;
            m_player.m_state = l_playerStates[0];
            break;
        case NetworkLib::ServerMessageType::Reject:
            Log::Debug("Server full... Join failed");
            break;
        case NetworkLib::ServerMessageType::State:
            Log::Debug("New State!");
            iar >> numPlayers;
            PlayerState l_state;
            for (int i = 0; i < numPlayers ; i++)
            {
                iar >> l_state;
                l_playerStates.push_back(l_state);

                Log::Debug(numPlayers, l_state.x, l_state.y, l_state.facing, l_state.speed);
                
            }
            // Get player id and do stuff accordingly

            break;
        default:
            break;
        }
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
        
        Log::Debug(ret);
        break;
    default:
        break;
    }
    
    return ret;
}
