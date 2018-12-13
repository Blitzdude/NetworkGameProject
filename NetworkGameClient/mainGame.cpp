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
        
        NetworkLib::ServerMessageType l_type;
        iar >> l_type;
        switch (l_type)
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
                m_player.m_statePredictionHistory.emplace(l_receivedTick, l_receivedState );
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
            // Clear the players info on other players
            m_otherPlayers.clear();
            
            // TODO: make the client remove players no found (or send removal message from server)

            // Read the state package
            uint32 l_receivedNumberOfPlayers;
            iar >> l_receivedNumberOfPlayers;

            uint64 l_receivedTick;
            iar >> l_receivedTick;

            float32 l_receivedTimestamp; // most recent time stamp server had from client at the time of writing this package
            iar >> l_receivedTimestamp; // used to estimate roundTripTime
           
            // On receiving the state package, calculate where the client should predict to
            float32 l_rttSec = m_currentTime - l_receivedTimestamp ;
            uint32 l_ticksToPredict = TimeToTick(l_rttSec);
            l_ticksToPredict += 2; // Add a little for jitter. TODO: make better format for calculating jitter
            m_targetTickNumber = l_ticksToPredict + l_receivedTick;
            
            

            // First id-state pair in state package is for the local player
            uint32 l_receivedId;
            iar >> l_receivedId;

            PlayerState l_receivedState;
            iar >> l_receivedState;

            // On the first message or if server is ahead of us, 
            //set current tick to server tick and use it to calculate the time
            if (l_receivedTick >= m_currentTicks)
            {   
                Log::Debug("Server is ahead of us. Resetting time to: ", m_targetTickNumber);
                m_currentTicks = m_targetTickNumber;
                m_currentTime = TickToTime(m_currentTicks);
            }
            else
            {
                // Prune the prediction history for predictions older then the received tick
                while (!m_player.m_statePredictionHistory.empty() &&
                        m_player.m_statePredictionHistory.begin()->first < l_receivedTick)
                {
                    // Older elements are in front of the map
                    m_player.m_statePredictionHistory.erase(m_player.m_statePredictionHistory.begin());
                }

                // insert the new prediction to players prediction history
                auto l_insertedState = m_player.m_statePredictionHistory.insert(std::make_pair(l_receivedTick, l_receivedState));
                if (l_insertedState.second) // second is false, if there was a value with key.
                {
                    // if a Key is already found, check for error within margin (0.01)

                    // calculate the deviations in x- and y-position between latestPrediction and received state
                    float32 dx = m_player.GetNewestState().second.x - l_receivedState.x;
                    float32 dy = m_player.GetNewestState().second.y - l_receivedState.y;
                    constexpr float32 c_maxError = 0.01f;
                    constexpr float32 c_maxErrorSqrd = c_maxError * c_maxError;
                    float32 l_errorSqrd = (dx * dx) + (dy * dy);
                    if (l_errorSqrd > c_maxError)
                    {
                        // if error is found, Misprediction has happened
                        Log::Debug("Misprediction error of: ", sqrtf(l_errorSqrd), " has occurred at tick: ", l_receivedTick, " Rewinding and replaying");
                        // change the value at tick-index (l_insertedState)
                        m_player.m_statePredictionHistory[l_receivedTick] = l_receivedState;
                        // remove all values older then changed tick

                        // simulate back to the latest tick-state
                        for (auto itr = m_player.m_statePredictionHistory.find(l_receivedId);
                                std::next(itr) != m_player.m_statePredictionHistory.end(); 
                                itr++)
                        {
                            auto l_loopCurrentState = itr;
                            uint32 l_loopCurrentTick = itr->first;
                            // get the input the player had for the fixable state
                            auto l_loopCurrentInput = m_player.m_inputPredictionHistory.find(itr->first);

                            // next state to be modified
                            auto l_nextState = std::next(itr);

                            // TODO: Need deltaTime, use difference of ticks?
                            // TODO: LEFTOFF: implement fixing of prediction history.
                            l_nextState->second = 
                                m_player.CalculateNewState(l_loopCurrentState->second, 
                                                           l_loopCurrentInput->second, 
                                                           TickToTime(l_nextState->first - l_loopCurrentState->first));
                        }

                        // Handle any Mispredictions, by simulating back to the present from the corrected state
                    }
                    
                }

                
            }

            // Get the remaining states for other players
            uint32 l_OtherId;
            PlayerState l_otherState;
            for (unsigned int i = 1; i < l_receivedNumberOfPlayers; ++i)
            {
                iar >> l_OtherId;
                iar >> l_otherState;
                auto itr = m_otherPlayers.find(l_OtherId);
                if ( itr != m_otherPlayers.end())
                {   
                    itr->second = l_otherState;
                }
                else
                {
                    m_otherPlayers.insert(std::make_pair(l_OtherId, l_otherState));
                }
            }
            break;
        }
        default:
            // Should never come here, assert if does
            Log::Error("Message type not found", (uint8)l_type);
            assert(0);
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
        // TODO: Implement: SerializeLeavePackage();
        l_archive << (uint8)NetworkLib::ClientMessageType::Leave;
        m_connection.Send(oss.str());
    }


    return true;
}

void MainGame::Update(float fElapsedTime)
{
    // TODO: move below to player update and just call this instead
    // m_player.Update(float fElapsedTime);

    m_player.m_previousInput = m_player.m_input;
    m_player.m_input = {false,false,false,false};

    m_player.m_currentState = m_player.GetNewestState().second;
    
    // update player
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

}

void MainGame::Draw()
{

    std::string toDraw = "Player Id: " + std::to_string(m_player.m_id);
    DrawString(0,0, toDraw, olc::WHITE, 4U);
    // draw player
    if (GameState::Joined == m_gameState)
    {
    PlayerState l_currentPlayerState = m_player.GetNewestState().second;

    DrawCircle((int32_t)l_currentPlayerState.x, (int32_t)l_currentPlayerState.y, 10);

    DrawLine(l_currentPlayerState.x, l_currentPlayerState.y,
        l_currentPlayerState.x + cosf(l_currentPlayerState.facing) * 10.0f,
        l_currentPlayerState.y + sinf(l_currentPlayerState.facing) * 10.0f, olc::MAGENTA);
    }

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

