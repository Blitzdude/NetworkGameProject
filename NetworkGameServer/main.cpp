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
        l_gm.m_totalTime += l_fElapsedTime;
        

        while (l_server.HasMessages())
        {
            // first part is the client id, second is the message
            auto l_msg = l_server.PopMessage();
            
            Log::Debug(l_msg.first, "size; ", l_msg.second);

            std::istringstream iss(l_msg.first);    
            boost::archive::text_iarchive iar(iss);


            NetworkLib::ClientMessageType l_type;
            iar >> l_type;

            
            switch (l_type)
            {
            case NetworkLib::ClientMessageType::Join:
            {

                // add player to list of players
                PlayerState l_np = { 330.0f,330.0f,0.0f, 10.0f };
                uint32 l_npId = l_gm.m_numPlayers++;
                l_gm.m_playerStates.emplace(l_npId, l_np);

                Log::Debug("Player Joined: ", l_npId);
                // send player their starting position
                l_server.SendToClient(l_gm.SerializeAcceptPackage(l_np, l_npId), l_msg.second);
                break;
            }
            case NetworkLib::ClientMessageType::Leave:
            {
                // Remove the player from player states
                break;
            }
            case NetworkLib::ClientMessageType::Input:
            {
                Log::Debug("Player input get!");
                uint32 l_id;
                iar >> l_id;
                PlayerInput l_input;
                iar >> l_input;

                float64 l_time;
                iar >> l_time;
                Log::Debug("Input:",l_id, l_input.up, l_input.down, l_input.left, l_input.right);

                // update client position on server
                l_gm.UpdateState(l_input, l_id, l_fElapsedTime);
                // Send server package to clients
                for (auto itr : l_gm.m_playerStates)
                {
                    l_server.SendToClient(l_gm.SerializeStatePackage(itr.first), l_msg.second);
                }

                break;
            }
            default:
                break;
            }
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