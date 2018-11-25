#pragma once

#include <NetworkLib/Constants.h>

struct PlayerInput
{
    bool32 up,down,left,right;
};

struct PlayerState
{
    float x, y, facing, speed; // Facing in radians
};

class Player
{
private:
public:
    Player();
    ~Player();

    bool32 HasInput();
private:
public:
    PlayerState m_state;
    PlayerInput m_input;

};
