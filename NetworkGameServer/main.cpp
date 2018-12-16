#include <iostream>
#include <list>
#include <chrono>
#include <NetworkLib/Server.h>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>
#include <common/tagOrDieCommon.h>
#include <NetworkLib/Statistics.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include "GameManager.h"

int main(int argc, char* argv[])
{
    
    if (argc != 2)
    {
        std::cout << "Usage: Network <port>\n";
        return 1;
    }
    NetworkLib::Server l_server(boost::lexical_cast<int>(argv[1]));
    GameManager l_gm;
    l_gm.m_currentTick = 0;

    bool isRunning = true;
    while (isRunning)
    {
        // Handle timing

        l_gm.m_timer.Restart();
        while (l_server.HasMessages())
        {
            // first part is the client id, second is the message
            auto l_msg = l_server.PopMessage();
            
            std::istringstream iss(l_msg.first);    
            boost::archive::text_iarchive iar(iss);

            NetworkLib::ClientMessageType l_type;
            iar >> l_type;

            switch (l_type)
            {
            case NetworkLib::ClientMessageType::Join:
            {
                // add player to list of players
                PlayerState l_npState = { 200.0f * l_gm.m_numPlayers ,300.0f, 0.0f};
                auto l_np = l_gm.AddPlayer(l_npState, l_msg.second);
                if (l_np.second)
                {
                    // new player inserted
                    Log::Debug("Player Joined: ", l_np.first);
                    l_server.SendToClient(l_gm.SerializeAcceptPackage(l_npState, l_np.first),
                                          l_gm.m_playerEndpointIds[l_np.first]);

                    l_gm.SendStateToAllClients(l_server);
                }
                else
                {
                    // no room, or failed otherwise
                    Log::Debug("Player Rejected: ", l_np.first);
                    l_server.SendToClient(l_gm.SerializeRejectPackage(), l_msg.second);
                }
                
                break;
            }
            case NetworkLib::ClientMessageType::Leave:
            {
                l_gm.RemovePlayerByEndpoint(l_msg.second);
                l_gm.SendStateToAllClients(l_server);

                break;
            }
            case NetworkLib::ClientMessageType::Input:
            {
                uint32 l_receivedId;
                iar >> l_receivedId;
                // timestamp received from client, used to estimate RTT.
                float32 l_receivedTimestamp;
                iar >> l_receivedTimestamp; 

                uint32 l_receivedTick;
                iar >> l_receivedTick;

                PlayerInput l_receivedInput;
                iar >> l_receivedInput;
                // the input from player needs to be from the future
                if (l_receivedTick >= l_gm.m_currentTick)
                {
                    const uint32 c_max_input_buffer_capacity = c_ticks_per_second * 2;
                    if (l_receivedTick - l_gm.m_currentTick < c_max_input_buffer_capacity)
                    {
                        // add tick to multimap, including the pair
                        l_gm.m_inputBuffer.emplace(l_receivedTick, 
                                                    std::make_pair(l_receivedId, l_receivedInput));
                    }
                    else
                    {
                        Log::Debug("Input ignored. Too far ahead: ", l_receivedTick,
                            " we are at: ", l_gm.m_currentTick, 
                            " difference of: ", l_receivedTick - l_gm.m_currentTick);
                   
                    }
                }
                else
                {
                    Log::Debug("Input ignored. behind: ", l_receivedTick,
                        " we are at: ", l_gm.m_currentTick, 
                        " difference of: ", l_gm.m_currentTick - l_receivedTick);
                
                }

                l_gm.m_clientTimeStamps[l_receivedId] = l_receivedTimestamp;

                break;
            }
            default:
                // Should never come here, assert if it does
                Log::Error("Message type not found", (uint8)l_type);
                assert(0);
                break;
            }
        }

        // update client inputs from the input buffers
        // get all input pairs for current tick
        auto l_range = l_gm.m_inputBuffer.equal_range(l_gm.m_currentTick);
        for (auto itr = l_range.first; itr != l_range.second; ++itr)
        {
            // for each pair, set the current input for the current player
            l_gm.m_playerInputs[itr->second.first] = itr->second.second;
        }
        
        // now that inputs for range of tick have been applied, remove all
        // elements with key older then current tick
        l_gm.m_inputBuffer.erase(l_gm.m_inputBuffer.begin(), l_range.second);

        // update client position on server
        for (auto &itr : l_gm.m_playerStates)
        {
            itr.second = l_gm.Tick(itr.second, l_gm.m_playerInputs.at(itr.first));
        }

        // Send game state server package to clients 10 times a second        
        if (l_gm.m_currentTick >= l_gm.m_lastTickStatesSent + (c_ticks_per_second / c_packages_per_second))
        {
            l_gm.SendStateToAllClients(l_server);
            l_gm.m_lastTickStatesSent = l_gm.m_currentTick;
        }

        l_gm.m_currentTick++;
        // caps the framerate to number of ticks (default 60)
        l_gm.m_timer.WaitUntilNextTick();
    } 

    // Average byte size of message
    if (l_server.GetStatistics().GetReceivedMessages() != 0) // stop division by zero error
    {
        Log::Info("Average bytes: "
            , l_server.GetStatistics().GetReceivedBytes()
            , "//", l_server.GetStatistics().GetReceivedMessages()
            , "=", l_server.GetStatistics().GetReceivedBytes()
            / l_server.GetStatistics().GetReceivedMessages());
    }
    
    return 0;
}