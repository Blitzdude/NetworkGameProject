#pragma once

#include <NetworkLib/Constants.h>
#include <map>
#include <NetworkLib/Messages.h>
#include <common/tagOrDieCommon.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

class Player
{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& p_archive, const unsigned int p_version)
    {
        p_archive & NetworkLib::ClientMessageType::Input;
        p_archive & m_id;
        p_archive & m_input;
        p_archive & m_playerTime;
    }

public:
    Player();
    ~Player();

    bool HasInput();

    // Serialize input
    std::string SerializeInput();

    // Returns the state with the largest tick number
    std::pair<uint32, PlayerState> GetNewestState();
    void InsertState(const PlayerState& state, uint32 tick);

private:
public:
    
    // map container with pairs of tickNumber-PlayerState, latest input should be the one with highest tick
    std::map<uint32, PlayerState> m_states; 
    std::map<uint32, PlayerState> m_otherPlayerStates;
    PlayerState m_currentState;
    PlayerInput m_input;
    PlayerInput m_previousInput;
    uint32 m_id = 0;

    float64 m_playerTime;
};
