#pragma once

#include <cinttypes>

namespace definitions
{
constexpr static uint16_t NR_QUEUES       = 1;
constexpr static uint8_t SELECTED_QUEUE   = 0;
constexpr static auto RING_SIZE           = 4096;
constexpr static auto PACKET_SIZE         = 1472;
constexpr static auto PAYLOAD_SIZE        = 1464;//1472 is max standard UDP payload - 8 byte hrzr
constexpr static auto SAMPLE_SIZE_IN_BYTE = 4;

constexpr static auto SAMPLES_PER_PACKET = PAYLOAD_SIZE / SAMPLE_SIZE_IN_BYTE;
}// namespace definitions
