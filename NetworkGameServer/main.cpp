#include <iostream>
#include <NetworkLib/Server.h>
#include <NetworkLib/Messages.h>
#include <NetworkLib/Log.h>

int main(int argc, char* argv[])
{
    NetworkLib::Server l_server(8080);

    bool isRunning = true;
    while (isRunning)
    {
        if (l_server.GetClientCount() > 0)
        {
            while (l_server.HasMessages())
            {
                auto l_msg = l_server.PopMessage();

                std::cout << l_msg.first << std::endl;

                std::string l_toClient = "from Server: ";
                l_toClient.append(l_msg.first);

                l_server.SendToAllExcept(l_toClient, l_msg.second);
            }
        }
    }

}