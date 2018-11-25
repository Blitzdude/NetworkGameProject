#include <iostream>
#include <string>

#include "mainGame.h"
int main(int argc, char* argv[])
{
 
    MainGame app;
    if (app.Construct(800, 600, 1, 1))
        app.Start();

    return 0;
}