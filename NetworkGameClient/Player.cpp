#include "Player.h"
#include <NetworkLib/Messages.h>


Player::Player()
    : m_input({false,false,false,false})
    , m_state({0.0f, 0.0f, 0.0f, 0.0f})
{
    
}


Player::~Player()
{
}

bool32 Player::HasInput()
{
    return (m_input.up || m_input.down || m_input.left || m_input.right);
}

/* input state packet*/
void Player::WriteInputPacket(uint8 * buffer)
{
    buffer[0] = (uint8)NetworkLib::ClientMessageType::Input;
    uint32 bytes_written = 1;

    memcpy(&buffer[bytes_written], &m_id, sizeof(m_id));
    bytes_written += sizeof(m_id);

    uint8 packed_input = (uint8)m_input.up ? 1 : 0 |
                     (uint8)m_input.down ?  (1 << 1) : 0 |
                     (uint8)m_input.left ?  (1 << 2) : 0 |
                     (uint8)m_input.right ? (1 << 3) : 0;

    buffer[bytes_written] = packed_input;
    ++bytes_written;

}
