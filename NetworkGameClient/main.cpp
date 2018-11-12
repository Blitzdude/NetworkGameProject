
#include "mainGame.h"

int main()
{
    MainGame app;
    if (app.Construct(800, 600, 1, 1))
        app.Start();

    return 0;
}