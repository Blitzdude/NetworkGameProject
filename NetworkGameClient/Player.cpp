#include "Player.h"
#include <sstream>
#include <NetworkLib/Messages.h>
#include "mainGame.h"

Player::Player()
    : m_input({false,false,false,false})
    , m_currentState({0.0f,0.0f,0.0f})
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

std::string Player::SerializeInput(float32 time, uint32 tick)
{

    std::ostringstream oss;
    boost::archive::text_oarchive l_archive(oss);
    
    l_archive << (uint8)NetworkLib::ClientMessageType::Input << m_id << time << tick << m_input;

    return oss.str();
}

// Newest state is at the back, hence rbegin()
std::pair<uint32, PlayerState> Player::GetNewestState()
{
    auto itr = m_statePredictionHistory.rbegin();
    std::pair<uint32, PlayerState> ret = std::make_pair(itr->first, itr->second);
    return ret;
}

// oldest state is at the front, hence begin()
std::pair<uint32, PlayerState> Player::GetOldestState()
{
    auto itr = m_statePredictionHistory.begin();
    std::pair<uint32, PlayerState> ret = std::make_pair(itr->first, itr->second);
    return ret;
}

PlayerState Player::Tick(const PlayerState& state, const PlayerInput& input)
{
    PlayerState l_ret = state;
    if (input.up)
    {
        l_ret.x += cosf(l_ret.facing) * c_speed * c_seconds_per_tick;
        l_ret.y += sinf(l_ret.facing) * c_speed * c_seconds_per_tick;
    }
    if (input.down)
    {
        l_ret.x -= cosf(l_ret.facing) * c_speed * c_seconds_per_tick;
        l_ret.y -= sinf(l_ret.facing) * c_speed * c_seconds_per_tick;
    }
    if (input.left)
    {
        l_ret.facing += c_turn_speed * c_seconds_per_tick;
    }
    if (input.right)
    {
        l_ret.facing -= c_turn_speed * c_seconds_per_tick;
    }

    return l_ret;
}
// ticks the player, until target Tick is reached
void Player::Update(uint32 targetTick)
{   
    uint32 l_newestTick = GetNewestState().first;
    // TODO: slow, or speed up client instead of calculating all states
    if (l_newestTick < targetTick)
    {
        // if the map is too large, remove the last element
        if ( m_statePredictionHistory.size() > c_ticks_per_second)
        {
            // Older elements are in front of the map
            m_statePredictionHistory.erase(m_statePredictionHistory.begin());
        }

        // predict the tick and add a new state to state prediction buffer 
        l_newestTick++;
        PlayerState l_newState = Tick(GetNewestState().second, m_input);
        m_statePredictionHistory.emplace(l_newestTick, l_newState);
        m_inputPredictionHistory.emplace(l_newestTick, m_input);
    }
}


