#include "Player.h"



Player::Player()
    : m_input({false,false,false,false})
    , m_state({0.0f, 0.0f, 0.0f, 0.0f})
{
    
}


Player::~Player()
{
}

bool32 Player::HasInput()
{
    return (m_input.up || m_input.down || m_input.left || m_input.right);
}
