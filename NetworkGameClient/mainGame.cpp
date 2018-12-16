#include "mainGame.h"
#include <boost/asio.hpp>
#include <NetworkLib/Constants.h>
#include <NetworkLib/Log.h>
#include <common/Timer.h>

/*
* Tag Or Die - client
* Client states:
* - Joining
* - Joined
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

bool MainGame::OnUserUpdate(float) 
{
    m_currentTime += m_timer.GetDeltaSeconds();
    m_timer.Restart();

    

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
                    m_currentTime = m_timer.TickToTime(m_currentTicks);
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
            
            // Read the state package
            uint32 l_receivedNumberOfPlayers;
            iar >> l_receivedNumberOfPlayers;

            uint64 l_receivedTick;
            iar >> l_receivedTick;

            float32 l_receivedTimestamp; // most recent time stamp server had from client at the time of writing this package
            iar >> l_receivedTimestamp; // used to estimate roundTripTime
           
            // On receiving the state package, calculate where the client should predict to
            float32 l_rttSec = m_currentTime - l_receivedTimestamp ;
            uint64 l_ticksToPredict = m_timer.TimeToTick(l_rttSec);
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
            
               // Log::Debug("Server is ahead of us. Resetting time to: ", m_targetTickNumber);
            
                m_currentTicks = m_targetTickNumber;
                m_currentTime = m_timer.TickToTime(m_currentTicks);
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
                //auto l_insertedState = m_player.m_statePredictionHistory.insert(std::make_pair(l_receivedTick, l_receivedState));
                if (m_player.m_statePredictionHistory.find(l_receivedTick) != m_player.m_statePredictionHistory.end()) // second is false, if there was a value with key.
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
                        for (auto itr = m_player.m_statePredictionHistory.find(l_receivedTick);
                                std::next(itr) != m_player.m_statePredictionHistory.end();
                                itr++)
                        {

                            auto l_loopCurrentState = itr;
                            uint64 l_loopCurrentTick = itr->first;
                            // get the input the player had for the fixable state
                            auto l_loopCurrentInput = m_player.m_inputPredictionHistory.find(itr->first);

                            // next state to be modified
                            auto l_nextState = std::next(itr);

                            m_player.m_statePredictionHistory[l_nextState->first] =
                                m_player.Tick(l_loopCurrentState->second, l_loopCurrentInput->second );
                        }
                    }
                }
                else
                {
                    // no prediction found
                    m_player.m_statePredictionHistory.emplace(l_receivedTick, l_receivedState);
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
                    itr->second.first = l_receivedTick; // tick last seen
                    itr->second.second = l_otherState; // last state
                }
                else
                {
                    m_otherPlayers.emplace(l_OtherId, std::make_pair(l_receivedTick, l_otherState));
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

    if (GameState::Joined == m_gameState)
    {
        Update();

        if (m_player.HasInput())
        {
            m_connection.Send(m_player.SerializeInput(m_currentTime, m_currentTicks));
        }
    }

    Draw();
    m_timer.WaitUntilNextTick();

    return isRunning; // if false -> exits program
}

bool MainGame::OnUserDestroy()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    if (m_gameState != GameState::Disconnected)
    {
        // serialize Leave package |msgType|
        // TODO: Implement: SerializeLeavePackage();
        l_archive << (uint8)NetworkLib::ClientMessageType::Leave;
        m_connection.Send(oss.str());
    }
    return true;
}

void MainGame::Update()
{

    // update player input
    m_player.m_previousInput = m_player.m_input;
    m_player.m_input = {false,false,false,false};
    if (GetKey(olc::W).bHeld) // forward
    {
        m_player.m_input.up = true;
    }
    if (GetKey(olc::S).bHeld) // back
    {
        m_player.m_input.down = true;
    }
    if (GetKey(olc::D).bHeld) // turn left
    {
        m_player.m_input.left = true;
    }
    if (GetKey(olc::A).bHeld) // turn right
    {
        m_player.m_input.right = true;
    }
    m_player.Update(m_targetTickNumber);
    m_targetTickNumber++;
    // TODO: we are not necessaricly at the newest state
    m_player.m_currentState = m_player.GetNewestState().second;
}

void MainGame::Draw()
{
    Clear(olc::BLACK);
    
    // warning C4244 disabled: possible loss of data. We don't care about losing fractions when drawing
    #pragma warning(disable:4244)
    if (GameState::Joined == m_gameState)
    {
        PlayerState l_currentPlayerState = m_player.GetNewestState().second;

        std::string toDraw = "Player Id: " + std::to_string(m_player.m_id) + " " 
                            + std::to_string(l_currentPlayerState.x) 
                            + " : " + std::to_string(l_currentPlayerState.y)
                            + " : " + std::to_string(l_currentPlayerState.facing);


        DrawString(0,0, toDraw, olc::WHITE, 1);

        std::string fps = "FPS: : " + std::to_string(m_timer.GetFPS());
        DrawString(0, 80, fps, olc::WHITE, 4);

        // draw local player

            DrawCircle((int32_t)l_currentPlayerState.x, (int32_t)l_currentPlayerState.y, 10);

            DrawLine(l_currentPlayerState.x, l_currentPlayerState.y,
                l_currentPlayerState.x + cosf(l_currentPlayerState.facing) * 10.0f,
                l_currentPlayerState.y + sinf(l_currentPlayerState.facing) * 10.0f, olc::MAGENTA);

        // Draw other players
        for (auto itr : m_otherPlayers)
        {
            auto l_state = itr.second.second;
            DrawCircle(l_state.x, l_state.y, 10, olc::CYAN);
            DrawLine(l_state.x, l_state.y,
            l_state.x + cosf(l_state.facing) + cosf(l_state.facing) * 10.0f,
            l_state.y + sinf(l_state.facing) + sinf(l_state.facing) * 10.0f, olc::MAGENTA);
        }
    }
    // Re-enable warning C4244
    #pragma warning(default:4244)
}
