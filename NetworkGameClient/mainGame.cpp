#include "mainGame.h"
#include <boost/asio.hpp>
#include <NetworkLib/Constants.h>
#include <NetworkLib/Log.h>
#include <common/Timer.h>

#include <algorithm>

/*
* Tag Or Die - client
* Client states:
* - Joining
* - Joined
* - Disconnected
*/
bool MainGame::OnUserCreate() 
{
    m_connection.Send(SerializeJoinPackage());

    return true;
}

bool MainGame::OnUserUpdate(float) 
{
    // DeltaTime not used, just called to update fps-tracking
    float32 l_deltaTime = m_timer.GetDeltaSeconds();
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
                uint32 l_receivedSlotId;
                iar >> l_receivedSlotId;

                Log::Debug("Joining game ", l_receivedSlotId);

                m_player.m_id = l_receivedSlotId;
                Log::Debug(l_receivedSlotId);
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
            m_isRunning = false;
            break;
        }
        case NetworkLib::ServerMessageType::State:
        {
            // Reset deltaCounter for lerping
            m_deltaLerp = 0;
            // Read the state package
            uint32 l_receivedNumberOfPlayers;
            iar >> l_receivedNumberOfPlayers;

            uint32 l_receivedTick;
            iar >> l_receivedTick;

            float32 l_receivedTimestamp; // most recent time stamp server had from client at the time of writing this package
            iar >> l_receivedTimestamp; // used to estimate roundTripTime
           
            // On receiving the state package, calculate where the client should predict to
            float32 l_rttSec = m_timer.GetElapsedSeconds() - l_receivedTimestamp ;
            uint32 l_ticksToPredict = m_timer.TimeToTick(l_rttSec);
            l_ticksToPredict += 2 ; // Add a little for jitter. TODO: make better format for calculating jitter
            m_targetTickNumber = l_ticksToPredict + l_receivedTick;
            

            // First id-state pair in state package is for the local player
            uint32 l_receivedId;
            iar >> l_receivedId;

            PlayerState l_receivedState;
            iar >> l_receivedState;

            // record local players server state for ghosting
            m_localPlayerServerState = l_receivedState;
            // On the first message or if server is ahead of us, 
            //set current tick to server tick and use it to calculate the time
            if (l_receivedTick >= m_currentTicks)
            {
                Log::Debug("Server is ahead of us. Resetting time to: ", m_targetTickNumber);
            
                m_currentTicks = m_targetTickNumber;
                // m_currentTime = m_timer.TickToTime(m_currentTicks);
                m_player.m_statePredictionHistory.emplace(l_receivedTick, l_receivedState);
            }
        
            // Prune the prediction history for predictions older then the received tick
            while (!m_player.m_statePredictionHistory.empty() &&
                    m_player.m_statePredictionHistory.begin()->first < l_receivedTick)
            {
                // Older elements are in front of the map
                m_player.m_statePredictionHistory.erase(m_player.m_statePredictionHistory.begin());
            }

            // insert the new prediction to players prediction history
            if (m_player.m_statePredictionHistory.find(l_receivedTick) != m_player.m_statePredictionHistory.end())  {
                // if a Key is already found, check for error within margin (0.01)

                // calculate the deviations in x- and y-position between latestPrediction and received state
                float32 dx = m_player.GetNewestState().second.x - l_receivedState.x;
                float32 dy = m_player.GetNewestState().second.y - l_receivedState.y;
                constexpr float32 c_maxError = 0.01f;
                constexpr float32 c_maxErrorSqrd = c_maxError * c_maxError;
                float32 l_errorSqrd = (dx * dx) + (dy * dy);
                if (l_errorSqrd > c_maxError)
                {
                    /*
                    Log::Debug("Misprediction error of: ", sqrtf(l_errorSqrd), " has occurred at tick: ", l_receivedTick, " Rewinding and replaying");
                    */
                    // change the value at tick-index (l_insertedState)
                    m_player.m_statePredictionHistory[l_receivedTick] = l_receivedState;
                    // remove all values older then changed tick

                    // simulate back to the latest tick-state
                    for (auto itr = m_player.m_statePredictionHistory.find(l_receivedTick);
                            std::next(itr) != m_player.m_statePredictionHistory.end();
                            itr++)
                    {

                        auto l_loopCurrentState = itr;
                        uint32 l_loopCurrentTick = itr->first;
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

            // TODO: Create a way to determine, which player left the game
            if (l_receivedNumberOfPlayers < m_otherPlayersLastKnownState.size() + 1)
            {
                // if the number of players was less then before, clear the containers related to other players
                Log::Debug("Player left the game");
                m_otherPlayersLastKnownState.clear();
                m_otherPlayersCurrentPosition.clear();
            }

            // Get the remaining states for other players
            for (unsigned int i = 1; i < l_receivedNumberOfPlayers; ++i)
            {
                uint32 l_OtherId;
                iar >> l_OtherId;
                
                PlayerState l_otherState;
                iar >> l_otherState;

                auto itr = m_otherPlayersLastKnownState.find(l_OtherId);
                if ( itr != m_otherPlayersLastKnownState.end())
                {   
                    // id found
                    m_otherPlayersLastKnownState[l_OtherId] = std::make_pair(l_receivedTick, l_otherState); 
                }
                else
                {
                    // no player with id yet, add it
                    m_otherPlayersLastKnownState.emplace(l_OtherId, std::make_pair(l_receivedTick, l_otherState));
                    m_otherPlayersCurrentPosition.emplace(l_OtherId, l_otherState);
                }
            }
            // were there less players then before
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
        m_connection.Send(m_player.SerializeInput(m_timer.GetElapsedSeconds(), m_currentTicks));
        Update();
    }

    Draw();
    m_timer.WaitUntilNextTick();

    return m_isRunning; // if false -> exits program
}

bool MainGame::OnUserDestroy()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    if (m_gameState != GameState::Disconnected)
    {
        m_connection.Send(SerializeLeavePackage());
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

    // Toggle debugDrawing
    if (GetKey(olc::F).bPressed)
    {
        m_drawDebug = m_drawDebug == true ? false : true;
    }

    // interpolate the other players states before update, so 
    // current tick will be closer
    m_deltaLerp++;
    for (auto& itr : m_otherPlayersCurrentPosition)
    {
        float32 t1 = static_cast<float32>(m_otherPlayersLastKnownState.at(itr.first).first);
        
        itr.second = LerpPlayerState(itr.second, m_otherPlayersLastKnownState.at(itr.first).second
            , t1 - c_packages_per_second
            , t1
            , (t1 - c_packages_per_second) + m_deltaLerp);
    }

    while (m_currentTicks < m_targetTickNumber)
    {
        m_player.Update(m_targetTickNumber);
        m_currentTicks++;
    }
    m_targetTickNumber++;
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
        // draw playerText
        std::string toDraw = "Player Id: " + std::to_string(m_player.m_id) + " " 
                            + std::to_string(l_currentPlayerState.x) 
                            + " : " + std::to_string(l_currentPlayerState.y)
                            + " : " + std::to_string(l_currentPlayerState.facing);

        DrawString(0,0, toDraw, olc::WHITE, 1);

        std::string fps = "FPS: : " + std::to_string(m_timer.GetFPS());
        DrawString(0, ScreenHeight() - 12, fps, olc::WHITE, 1);

        // Draw local players server location
        if (m_drawDebug)
        {
            DrawCircle((int32_t)m_localPlayerServerState.x, (int32_t)m_localPlayerServerState.y, 10, olc::RED);

            DrawLine(m_localPlayerServerState.x, m_localPlayerServerState.y,
                m_localPlayerServerState.x + cosf(m_localPlayerServerState.facing) * 10.0f,
                m_localPlayerServerState.y + sinf(m_localPlayerServerState.facing) * 10.0f, olc::DARK_RED);
        }

        // draw local players current position
        DrawCircle((int32_t)l_currentPlayerState.x, (int32_t)l_currentPlayerState.y, 10);

        DrawLine(l_currentPlayerState.x, l_currentPlayerState.y,
            l_currentPlayerState.x + cosf(l_currentPlayerState.facing) * 10.0f,
            l_currentPlayerState.y + sinf(l_currentPlayerState.facing) * 10.0f, olc::MAGENTA);

        // Draw where other players are lerping to
        if (m_drawDebug)
        {
            for (auto itr : m_otherPlayersLastKnownState)
            {
                auto l_state = itr.second.second;
                DrawCircle(l_state.x, l_state.y, 10, olc::GREEN);
                DrawLine(l_state.x, l_state.y,
                l_state.x + cosf(l_state.facing) + cosf(l_state.facing) * 10.0f,
                l_state.y + sinf(l_state.facing) + sinf(l_state.facing) * 10.0f, olc::DARK_GREEN);
            }
        }

        // Draw other players
        for (auto itr : m_otherPlayersCurrentPosition)
        {
            auto l_state = itr.second;
            DrawCircle(l_state.x, l_state.y, 10, olc::CYAN);
            DrawLine(l_state.x, l_state.y,
            l_state.x + cosf(l_state.facing) + cosf(l_state.facing) * 10.0f,
            l_state.y + sinf(l_state.facing) + sinf(l_state.facing) * 10.0f, olc::MAGENTA);
        }

    }
    // Re-enable warning C4244
    #pragma warning(default:4244)
}

PlayerState MainGame::LerpPlayerState(const PlayerState & a, const PlayerState & b, float32 t0, float32 t1, float32 t)
{
    PlayerState l_ret = a;
    float32 fraction = 1.0f;
    if (t1 - t0 != 0)
    {
        fraction = (t - t0) / (t1 - t0);
    }
    // clamp it, so 0.0 <= t  <= 1.0
    fraction = std::max(0.0f, std::min(fraction, 1.0f));
    // interpolate player position to that state
    l_ret.x = a.x + fraction * (b.x - a.x);
    l_ret.y = a.y + fraction * (b.y - a.y);
    l_ret.facing = a.facing + fraction * (b.facing - a.facing);

    return l_ret;
}

std::string MainGame::SerializeJoinPackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    // Send a Join Request to server
    l_archive << NetworkLib::ClientMessageType::Join;

    return oss.str();
}

std::string MainGame::SerializeLeavePackage()
{
    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    l_archive << (uint8)NetworkLib::ClientMessageType::Leave;
    return oss.str();
}
