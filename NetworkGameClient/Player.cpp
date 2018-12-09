#include "Player.h"
#include <sstream>
#include <NetworkLib/Messages.h>

Player::Player()
    : m_input({false,false,false,false})
    , m_currentState({0.0f,0.0f,0.0f,0.0f})
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

std::string Player::SerializeInput(float32 time)
{

    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    
    //  type: id: input: time
    l_archive << (uint8)NetworkLib::ClientMessageType::Input << m_id << m_input << time;

    return oss.str();
}

std::pair<uint64, PlayerState> Player::GetNewestState()
{
    auto itr = m_predictionHistory.rbegin();
    std::pair<uint64, PlayerState> ret = std::make_pair(itr->first, itr->second);
    return ret;
}

std::pair<uint64, PlayerState> Player::GetOldestState()
{
    auto itr = m_predictionHistory.begin();
    std::pair<uint64, PlayerState> ret = std::make_pair(itr->first, itr->second);
    return ret;
}

void Player::InsertState(const PlayerState & state, uint64 tick)
{
    m_predictionHistory.insert(std::make_pair(tick ,state));
}

