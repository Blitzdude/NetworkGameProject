#pragma once

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef int int32;
typedef short int16;
typedef char int8;
typedef int32 bool32;
typedef float float32;
typedef double float64;
typedef int bool32;


static const int NetworkBufferSize = 1024;

// constants for players
constexpr float32 c_turn_speed = 1.0f;
// TODO: not needed constexpr float32 c_acceleration = 10.0f;
constexpr float32 c_max_speed = 150.0f;

constexpr uint64 packages_per_second = 10;
constexpr uint64 ticks_per_second = 60;
constexpr float32 seconds_per_tick = 1.0f / (float32)ticks_per_second;
constexpr uint16 TOD_MAX_CLIENTS = 4;
constexpr uint16 TOD_PORT = 8080;

