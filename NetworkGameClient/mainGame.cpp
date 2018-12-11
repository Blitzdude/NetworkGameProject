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
    m_currentTime += fElapsedTime;
    m_currentTicks = GetCurrentTick();
    // called once per frame
    // std::ostringstream oss;
    // boost::archive::text_oarchive l_oar(oss);

    Clear(olc::BLACK);

    if (GameState::Joined == m_gameState)
        Update(fElapsedTime);

    if (m_player.HasInput())
    {
        m_connection.Send(m_player.SerializeInput(m_currentTime));
    }

    while (m_connection.HasMessages())
    {
        auto l_msg = m_connection.PopMessage();

        std::istringstream iss(l_msg);
        boost::archive::text_iarchive iar(iss);
        
        NetworkLib::ServerMessageType type;
        iar >> type;
        switch (type)
        {
        case NetworkLib::ServerMessageType::Accept:
        {

            if (m_gameState == GameState::Joining)
            {
                // type id tick playestate
                PlayerState l_receivedState; // state of the player according to server
                
                uint32 l_receivedSlotId;
                iar >> l_receivedSlotId;
                uint64 l_receivedTick;
                iar >> l_receivedTick;

                if (l_receivedTick >= m_currentTicks)
                {
                    m_currentTicks = l_receivedTick;
                    m_currentTime = TickToTime(m_currentTicks);
                }

                iar >> l_receivedState;
                Log::Debug("Joining game ", l_receivedSlotId);

                m_player.m_id = l_receivedSlotId;
                m_player.InsertState(l_receivedState, l_receivedTick);
                Log::Debug(l_receivedSlotId, l_receivedTick);
                m_gameState = GameState::Joined;
            }
            else
            {
                Log::Error("Already Joined the game");
            }

            break;
        }
        case NetworkLib::ServerMessageType::Reject:
        {
            Log::Debug("Server full... Join failed");
            m_gameState = GameState::Disconnected;
            isRunning = false;
            break;
        }
        case NetworkLib::ServerMessageType::State:
        {
            
            // Read the state package
            uint32 l_receivedNumberOfPlayers;
            iar >> l_receivedNumberOfPlayers;

            uint64 l_receivedTick;
            iar >> l_receivedTick;

            float32 l_receivedTimestamp; // most recent time stamp server had from client at the time of writing this package
            iar >> l_receivedTimestamp;
            // Calculate round-trip Time and target tick;
            float32 l_rttSec = m_currentTime - l_receivedTimestamp ;
            uint64 l_targetTick = TimeToTick(l_rttSec) + m_currentTicks;
            l_targetTick += 2; // Add a little for jitter. TODO: make better format for calculating jitter
            //Log::Debug("State: ", l_receivedNumberOfPlayers, l_receivedTick, l_receivedTimestamp);
            // if server is ahead of us, set current tick to server tick and use it to calculate the time
            if (l_receivedTick >= m_currentTicks)
            {
                m_currentTicks = l_receivedTick;
                m_currentTime = l_targetTick;
            }
            else
            {
                // remove un-needed elements from the map (for 2 seconds)
                while (m_player.m_predictionHistory.size() > ticks_per_second * 2)
                {
                    m_player.m_predictionHistory.erase(m_player.m_predictionHistory.begin());
                }
                // insert the new prediction to players prediction history
            }
            
            uint32 l_receivedId;
            iar >> l_receivedId;

            // First state in Server-state package is for the local player
            PlayerState l_receivedState; 
            iar >> l_receivedState;
            
            m_player.InsertState(l_receivedState, l_receivedTick);
            // ss peek?
            uint32 l_receivedOtherId;
            PlayerState l_otherState;
            for (unsigned int i = 1; i < l_receivedNumberOfPlayers; ++i)
            {
                iar >> l_receivedOtherId;
                iar >> l_otherState;
                auto itr = m_otherPlayers.find(l_receivedOtherId);
                if ( itr != m_otherPlayers.end())
                {   
                    itr->second = l_otherState;
                }
                else
                {
                    m_otherPlayers.insert(std::make_pair(l_receivedOtherId, l_otherState));
                }
            }
            // record the localPlayers state and the states of other players
            // Update the local and other players positions by fixing the historic buffer and interpolating between previous positions
            
            break;
        }
        default:
            break;
        }
    }
    Draw();

    return isRunning; // if false -> exits program
}

bool MainGame::OnUserDestroy()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    // Send a Leave Message to server TODO: send this about 10 times 
    if (m_gameState != GameState::Disconnected)
    {
        // serialize Leave package |msgType|
        l_archive << (uint8)NetworkLib::ClientMessageType::Leave;
    }

    m_connection.Send(oss.str());

    return true;
}

void MainGame::Update(float fElapsedTime)
{
    m_player.m_previousInput = m_player.m_input;
    m_player.m_input = {false,false,false,false};

    m_player.m_currentState = m_player.GetNewestState().second;
    


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

    std::string toDraw = "Player Id: " + std::to_string(m_player.m_id);
    DrawString(0,0, toDraw, olc::WHITE, 4U);
    // draw player
    DrawCircle((int32_t)m_player.m_currentState.x, (int32_t)m_player.m_currentState.y, 10);

    DrawLine(m_player.m_currentState.x, m_player.m_currentState.y,
        m_player.m_currentState.x + cosf(m_player.m_currentState.facing) * 10.0f,
        m_player.m_currentState.y + sinf(m_player.m_currentState.facing) * 10.0f, olc::MAGENTA);

    // Draw other players
    for (auto itr : m_otherPlayers)
    {
        DrawCircle(itr.second.x, itr.second.y, 10, olc::CYAN);
        DrawLine(itr.second.x, itr.second.y,
        itr.second.x + cosf(itr.second.facing) + cosf(itr.second.facing) * 10.0f,
        itr.second.y + sinf(itr.second.facing) + sinf(itr.second.facing) * 10.0f, olc::MAGENTA);
    }
    
    
}

float32 MainGame::TickToTime(uint64 tick)
{
    return tick * seconds_per_tick;
}

uint64 MainGame::TimeToTick(float32 time)
{
    return time * ticks_per_second;
}

uint64 MainGame::GetCurrentTick()
{
    return static_cast<uint64>(m_currentTime * ticks_per_second); // static casting to stop compiler warning
}

