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

    // Serialize input
    std::string SerializeInput(float32 time);

    // Returns the state with the largest tick number
    std::pair<uint64, PlayerState> GetNewestState();
    std::pair<uint64, PlayerState> GetOldestState();
    // returns false, if either state or input already exists for tick
    bool InsertState(const PlayerState& state, const PlayerInput& input, uint64 tick);
    // calculates next state based on state, input and difference in ticks
    PlayerState CalculateNewState(const PlayerState& state, const PlayerInput& input, float32 deltaTime);
    void Update(float deltaTime);

private:
public:
    // map container <tick, state>, begin()->oldest, back()->newest
    // TODO: encapsulate members
    std::map<uint64, PlayerState> m_statePredictionHistory;
    std::map<uint64, PlayerInput> m_inputPredictionHistory;
    //std::map<uint32, PlayerState> m_otherPlayerStates;
    PlayerState m_currentState;
    PlayerInput m_input;
    PlayerInput m_previousInput;
    uint32 m_id = 0;

};
