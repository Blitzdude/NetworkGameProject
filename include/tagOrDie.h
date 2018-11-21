#pragma once

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef int int32;
typedef short int16;
typedef char int8;
typedef int bool32;

constexpr uint16 TOD_MAX_CLIENTS = 4;
constexpr uint16 TOD_PORT = 9999;
constexpr uint32 TOD_SOCKET_BUFFER_SIZE = 1024;
constexpr uint16 TOD_NUMBER_OF_THREADS = 2;