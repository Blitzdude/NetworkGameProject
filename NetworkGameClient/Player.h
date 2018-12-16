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

    std::string SerializeInput(float32 time, uint64 tick);

    // Returns the state with the largest tick number
    std::pair<uint64, PlayerState> GetNewestState();
    std::pair<uint64, PlayerState> GetOldestState();
    // returns false, if either state or input already exists for tick
    bool InsertState(const PlayerState& state, const PlayerInput& input, uint64 tick);
    // calculates state for next tick based on state and input
    PlayerState Tick(const PlayerState& state, const PlayerInput& input);
    // ticks the player, until target Tick is reached
    void Update(uint64 targetTick ); 

public:
    // TODO: encapsulate members
    // map container <tick, state>, begin()->oldest, back()->newest
    std::map<uint64, PlayerState> m_statePredictionHistory;
    std::map<uint64, PlayerInput> m_inputPredictionHistory;
    PlayerState m_currentState;
    PlayerInput m_input;
    PlayerInput m_previousInput;
    uint32 m_id = 0;

};
