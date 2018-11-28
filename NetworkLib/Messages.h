#pragma once
#include <string>
#include "Constants.h"

namespace NetworkLib
{
enum class ClientMessageType : uint8
{
    Join = 'J',
    Leave = 'L',
    Input = 'I'
};
enum class ServerMessageType : uint8
{
    Accept  = 'A',
    Reject  = 'R',
    State   = 'S'
};
};

