#pragma once
#include <NetworkLib/Constants.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <iostream>
#include <vector>

struct PlayerInput
{
    bool32 up;
    bool32 down;
    bool32 left;
    bool32 right;
};

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, PlayerInput & p, const unsigned int version)
{
    ar & p.up;
    ar & p.down;
    ar & p.left;
    ar & p.right;
}

} // namespace serialization
} // namespace boost

struct PlayerState
{
    float x;
    float y;
    float facing; // Facing in radians
    float speed;
};

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, PlayerState & p, const unsigned int version)
{
    ar & p.x;
    ar & p.y;
    ar & p.facing;
    ar & p.speed;

}

} // namespace serialization
} // namespace boost
