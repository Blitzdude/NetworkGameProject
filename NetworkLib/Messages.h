#pragma once
#include <string>
#include "Constants.h"

namespace NetworkLib
{

enum class ClientMessageType : uint8
{
    Join,
    Leave,
    Input
};

enum class ServerMessageType : uint8
{
    Accept,
    Reject,
    State
};

};

