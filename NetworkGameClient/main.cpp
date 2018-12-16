#include <iostream>
#include <string>
#include "mainGame.h"
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: TagOrDie <ip-address> <port>\n";
        return 1;
    }
    
    
    MainGame app(argc, argv);
    if (app.Construct(800, 600, 1, 1))
        app.Start();
    
    // std::cin.get();
    return 0;
}