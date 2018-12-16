#pragma once
// This file is included in the commons file directory, thus visible to all projects
#include <NetworkLib/Constants.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <iostream>
#include <vector>

struct PlayerInput
{
    // TODO: This input can be packed into a single bool32;
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
    float32 x;
    float32 y;
    float32 facing; // Facing in radians
    // float32 speed;
};

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, PlayerState & p, const unsigned int version)
{
    ar & p.x;
    ar & p.y;
    ar & p.facing;
}

} // namespace serialization
} // namespace boost

/* TAG or DIE
// -------------------------------------------
TODO:
COMMON: 
- Move Timer and This header to NetworkLib files, easier that way
- InputState can be packed into a single bool32, decreasing
    the amount of packages sent form client to server
- rename variables to follow a common coding convention
CLIENT:
- Encapsulate members
- Client side prediction.
- Timeout, when No response from server is detected
SERVER:
- Encapsulate members
- When server closes, send close message to rest of players.
- Timeouts when no response from client is detected
NETWORKLIB:

// -------------------------------------------
   Package Structures
   --------------
   Client::Join
   |MsgType|
   - A player from current endpoint has joined
   - respond with Server::Accept
   - if no room for game (numPlayers > 4), respond with Server::Reject
   Client::Leave
   |MsgType|ID|
   - Client has sent a leave request
   - Server removes the player from the game
   Client::Input
   |MsgType|ID|TimeStamp(float)|InpuTick|Input|
   - TimeStamp is for checking estimating RTT
   - InputTick is used to tie input to certain game moment
   - |Input| = PlayerInput-Struct

   Server::Accept
   |MsgType|ID|Tick|PlayerState|
   - Delivers the player id and initial state of the player
   Server::Reject
   |MsgType|
   - MsgType is enough to tell the client of the server join failure
   Server::State
   - Example: num player equals 4
   |MsgType|NumPlayers|ServerTick|LPTimeStamp|LPID|LPState|OP1ID|OP1State|OP2ID|OP2State|OP3ID|OP3State|
   - LPState and OPXState are PlayerState-structs
   - First ID and PlayerState structs are meant for the receiving client
// -------------------------------------------


*/