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

public:
    Player();
    ~Player();

    bool HasInput();

    std::string SerializeInput(float32 time, uint32 tick);

    // Returns the state with the largest tick number
    std::pair<uint32, PlayerState> GetNewestState();
    std::pair<uint32, PlayerState> GetOldestState();
    // calculates state for next tick based on state and input
    PlayerState Tick(const PlayerState& state, const PlayerInput& input);
    // ticks the player, until target Tick is reached
    void Update(uint32 targetTick );

public:
    // TODO: encapsulate members
    // map container <tick, state>, begin()->oldest, back()->newest
    std::map<uint32, PlayerState> m_statePredictionHistory;
    std::map<uint32, PlayerInput> m_inputPredictionHistory;
    PlayerState m_currentState;
    PlayerInput m_input;
    PlayerInput m_previousInput;
    uint32 m_id = 0;

};
