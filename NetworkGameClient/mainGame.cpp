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
    m_totalTicks = GetCurrentTick();

    Log::Debug(m_totalTicks, m_totalTime);


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
        std::vector<std::pair<uint32, PlayerState>> l_otherPlayerStates; // states of all the other players <id, State>
        PlayerState l_localPlayerState; // state of the player according to server

        
        NetworkLib::ServerMessageType type;
        iar >> type;
        switch (type)
        {
        case NetworkLib::ServerMessageType::Accept:
            Log::Debug("Joining game");
            l_localPlayerState; // state of the player according to server

            iar >> l_localPlayerState;
            m_player.m_state = l_localPlayerState;
            break;
        case NetworkLib::ServerMessageType::Reject:
            Log::Debug("Server full... Join failed");
            break;
        case NetworkLib::ServerMessageType::State:
            Log::Debug("New State!");

            uint32 l_tickNumber;
            uint32 l_clientTimestamp; // most recent time stamp server had from client at the time of writing this package
            uint32 l_numberOfPlayers;
            uint32 l_tempId;
            PlayerState l_tempState; // for usage in for loop;


            // Read the state package
            // if server is ahead of us, set current tick to server tick and use it to calculate the time
            
            // record the localPlayers state and the states of other players
            // Update the local and other players positions by fixing the historic buffer and interpolating between previous positions
            iar >> l_numberOfPlayers;

            for (int i = 0; i < l_numberOfPlayers; i++)
            {
                iar >> l_tempId;
                iar >> l_tempState;

                if (l_tempId == m_player.m_id)
                {
                    m_player.m_state = l_tempState;
                }
                else
                {
                    l_otherPlayerStates.push_back(std::make_pair(l_tempId,l_tempState));
                }
            }

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
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    // Send a Join Request to server
    l_archive << NetworkLib::ClientMessageType::Leave;

    m_connection.Send(oss.str());

    return true;
}

void MainGame::Update(float fElapsedTime)
{
    m_player.m_previousInput = m_player.m_input;
   

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

uint64 MainGame::GetCurrentTick()
{
    return static_cast<uint64>(m_totalTime * ticks_per_second); // static casting to stop compiler warning
}

