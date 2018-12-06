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

    if (GameState::Joined == m_gameState)
        Update(fElapsedTime);

    if (m_player.HasInput())
    {
        m_connection.Send(m_player.SerializeInput());
    }

    while (m_connection.HasMessages())
    {
        auto l_msg = m_connection.PopMessage();
        Log::Debug(l_msg);
        std::istringstream iss(l_msg);
        boost::archive::text_iarchive iar(iss);
        // for network pacakges
        PlayerState l_localPlayerState; // state of the player according to server

        
        NetworkLib::ServerMessageType type;
        iar >> type;
        switch (type)
        {
        case NetworkLib::ServerMessageType::Accept:
            if (m_gameState == GameState::Joining)
            {
                Log::Debug("Joining game");
                l_localPlayerState; // state of the player according to server
                // type id tick playestate
                
                uint32 l_slotId;
                iar >> l_slotId;
                
                uint32 l_tick;
                iar >> l_tick;

                iar >> l_localPlayerState;

                m_player.m_id = l_slotId;
                m_player.m_states.insert(std::make_pair(l_tick, l_localPlayerState));
                Log::Debug(l_slotId, l_tick);
                m_gameState = GameState::Joined;
            }
            else
            {
                Log::Error("Already Joined the game");
            }

            break;
        case NetworkLib::ServerMessageType::Reject:
            Log::Debug("Server full... Join failed");
            m_gameState = GameState::Disconnected;
            break;
        case NetworkLib::ServerMessageType::State:
            Log::Debug("New State!");
            
            // First state in Server-state package is for the local player

            uint32 l_numberOfPlayers;
            uint32 l_tickNumber;
            float64 l_clientTimestamp; // most recent time stamp server had from client at the time of writing this package
            uint32 l_tempId;
            PlayerState l_tempState; // for usage in for loop;

            // Read the state package
            iar >> l_numberOfPlayers;
            iar >> l_tickNumber;
            iar >> l_clientTimestamp;
            iar >> l_tempId;
            iar >> l_tempState;

            m_player.m_states.insert(std::make_pair(l_tickNumber, l_tempState));

            for (int i = 0; i < l_numberOfPlayers - 1; i++)
            {
                iar >> l_tempId;
                iar >> l_tempState;
                auto it = m_otherPlayers.find(l_tempId);
                if (it != m_otherPlayers.end())
                {
                     m_otherPlayers[l_tempId] = l_tempState;
                }
            }

            // if server is ahead of us, set current tick to server tick and use it to calculate the time
            
            // record the localPlayers state and the states of other players
            // Update the local and other players positions by fixing the historic buffer and interpolating between previous positions
            
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
   
    m_player.m_currentState = m_player.m_states.begin()->second;
    // update player
    if (GetKey(olc::D).bHeld) // turn left
    {
        m_player.m_input.left = true;
        m_player.m_currentState.facing += 1.0f * fElapsedTime;
    }
    if (GetKey(olc::A).bHeld) // turn right
    {
        m_player.m_input.right = true;
        m_player.m_currentState.facing -= 1.0f * fElapsedTime;
    }
    if (GetKey(olc::W).bHeld) // forward
    {
        m_player.m_input.up = true;
        
        m_player.m_currentState.x += cosf(m_player.m_currentState.facing) * m_player.m_currentState.speed * fElapsedTime;
        m_player.m_currentState.y += sinf(m_player.m_currentState.facing) * m_player.m_currentState.speed * fElapsedTime;

    }
    if (GetKey(olc::S).bHeld) // back
    {
        m_player.m_input.down = true;

        m_player.m_currentState.x -= cosf(m_player.m_currentState.facing) * m_player.m_currentState.speed * fElapsedTime;
        m_player.m_currentState.y -= sinf(m_player.m_currentState.facing) * m_player.m_currentState.speed * fElapsedTime;
    }

}

void MainGame::Draw()
{
    float l_playerX = m_player.m_currentState.x;
    float l_playerY = m_player.m_currentState.y;
    float l_facing = m_player.m_currentState.facing;

    // draw player
    DrawCircle((int32_t)l_playerX, (int32_t)l_playerY, 10);

    DrawLine(m_player.m_currentState.x, m_player.m_currentState.y,
        m_player.m_currentState.x + cosf(l_facing) * 10.0f,
        m_player.m_currentState.y + sinf(l_facing) * 10.0f, olc::MAGENTA);
}

uint64 MainGame::GetCurrentTick()
{
    return static_cast<uint64>(m_totalTime * ticks_per_second); // static casting to stop compiler warning
}

