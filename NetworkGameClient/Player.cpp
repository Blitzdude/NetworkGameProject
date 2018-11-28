#include "Player.h"
#include <NetworkLib/Messages.h>

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

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, Player& pi, const unsigned int version)
{
    ar & NetworkLib::ClientMessageType::Input;
    ar & pi.m_id;
    ar & pi.m_input;
    ar & pi.m_playerTime;
}

} // !namespace serialization
} // !namespace boost

