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
    return (m_input.up    != m_previousInput.up     || 
            m_input.down  != m_previousInput.down   ||
            m_input.left  != m_previousInput.left   || 
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

// Newest state is at the back, hence rbegin()
std::pair<uint64, PlayerState> Player::GetNewestState()
{
    auto itr = m_statePredictionHistory.rbegin();
    std::pair<uint64, PlayerState> ret = std::make_pair(itr->first, itr->second);
    return ret;
}

// oldest state is at the front, hence begin()
std::pair<uint64, PlayerState> Player::GetOldestState()
{
    auto itr = m_statePredictionHistory.begin();
    std::pair<uint64, PlayerState> ret = std::make_pair(itr->first, itr->second);
    return ret;
}

bool Player::InsertState(const PlayerState & state, const PlayerInput & input, uint64 tick)
{
    bool success;
    success = m_statePredictionHistory.insert(std::make_pair(tick, state)).second;
    success = m_inputPredictionHistory.insert(std::make_pair(tick, input)).second;
    return success;
}

PlayerState Player::CalculateNewState(const PlayerState& state, const PlayerInput& input, uint64 deltaTicks)
{


    return PlayerState();
}


