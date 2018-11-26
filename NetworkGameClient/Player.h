#pragma once

#include <NetworkLib/Constants.h>

struct PlayerInput
{
    bool32 up,down,left,right;
};

struct PlayerState
{
    float x;
    float y;
    float facing; // Facing in radians
    float speed; 
};

class Player
{
private:
public:
    Player();
    ~Player();

    bool32 HasInput();

    void WriteInputPacket(uint8* buffer);
private:
public:
    PlayerState m_state;
    PlayerInput m_input;
    uint8 m_id = 0;
};
