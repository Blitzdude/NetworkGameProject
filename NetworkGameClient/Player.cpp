#include "Player.h"
#include <sstream>
#include <NetworkLib/Messages.h>

Player::Player()
    : m_input({false,false,false,false})
    , m_state({0.0f, 0.0f, 0.0f, 0.0f})
{
    
}

Player::~Player()
{
}

bool Player::HasInput()
{
    return (m_input.up != m_previousInput.up || 
            m_input.down != m_previousInput.down ||
            m_input.left != m_previousInput.left || 
            m_input.right != m_previousInput.right);
}

std::string Player::SerializeInput()
{

    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    
    //  type: id: input: time
    l_archive << (uint8)NetworkLib::ClientMessageType::Input << m_id << m_input << m_playerTime;

    return oss.str();
}

const const std::pair<uint32, PlayerState>& Player::GetNewestState()
{
    auto ret = m_states.begin();
    return std::make_pair(ret->first, ret->second);
}


void Player::InsertState(const PlayerState & state, uint32 tick)
{
    // find the element with 
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

