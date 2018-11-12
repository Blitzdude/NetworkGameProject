#pragma once

struct PlayerInput
{
    bool up,down,left,right;
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
private:
public:
    PlayerState m_state;
    PlayerInput m_input;

};

