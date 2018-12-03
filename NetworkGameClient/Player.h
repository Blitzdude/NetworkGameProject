#pragma once

#include <NetworkLib/Constants.h>
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

    bool32 HasInput();

    // Serialize input
    std::string SerializeInput();
private:
public:
    PlayerState m_state;
    PlayerInput m_input;
    uint8 m_id = 0;

    long double m_playerTime;
};
