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
#include "GameManager.h"


int main(int argc, char* argv[])
{
    NetworkLib::Server l_server(8080);
    GameManager l_gm;

    auto l_tp1 = std::chrono::system_clock::now();
    auto l_tp2 = std::chrono::system_clock::now();

    bool isRunning = true;
    while (isRunning)
    {
        // Handle timing
        l_tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = l_tp2 - l_tp1;
        l_tp1 = l_tp2;

        float32 l_fElapsedTime = elapsedTime.count();
        l_gm.m_currentTime += l_fElapsedTime;
        l_gm.m_currentTicks = l_gm.GetCurrentTick();


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
                uint32 l_npId = l_gm.m_numPlayers++;
                PlayerState l_npState = { 150.0f * l_gm.m_numPlayers ,300.0f,0.0f, 10.0f };
                l_gm.m_playerStates.emplace(l_npId, l_npState);
                
                // add new inputs slot
                PlayerInput l_npInput = {false, false,false,false};
                l_gm.m_playerInputs.emplace(l_npId, l_npInput);
                
                // record the slot and endpoint
                l_gm.m_playerEndpointIds.emplace(l_npId, l_msg.second);
                Log::Debug("Player Joined: ", l_npId);
                // send player their starting position
                l_server.SendToClient(l_gm.SerializeAcceptPackage(l_npState, l_npId),
                                      l_gm.m_playerEndpointIds[l_npId]);
                break;
            }
            case NetworkLib::ClientMessageType::Leave:
            {
                // Remove the player from player states
                uint32 l_id;
                iar >> l_id;
                l_gm.m_playerStates.erase(l_id);
                l_gm.m_numPlayers--;
                break;
            }
            case NetworkLib::ClientMessageType::Input:
            {
                Log::Debug("Player input get!");
                uint32 l_receivedId;
                iar >> l_receivedId;
                PlayerInput l_receivedInput;
                iar >> l_receivedInput;

                float32 l_receivedTimestamp;
                iar >> l_receivedTimestamp; // timestamp received from client, used to estimate RTT.

                auto l_ptr = l_gm.m_playerInputs.find(l_receivedId);
                if (l_gm.m_playerInputs.find(l_receivedId) != l_gm.m_playerInputs.end())
                {
                    l_gm.m_playerInputs[l_receivedId] = l_receivedInput;
                }
                
                // Send server package to clients
                for (auto itr : l_gm.m_playerStates)
                {
                    l_server.SendToClient(l_gm.SerializeStatePackage(itr.first), l_gm.m_playerEndpointIds.at(itr.first));
                }
                
                break;
            }
            default:
                break;
            }

        }

        // update client position on server
        for (auto itr : l_gm.m_playerStates)
        {
            l_gm.UpdateState(l_gm.m_playerInputs.at(itr.first), itr.first, l_fElapsedTime);
        }

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

    std::cin.get();
    return 0;
}