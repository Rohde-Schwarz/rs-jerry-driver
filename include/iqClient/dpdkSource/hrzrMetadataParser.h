#pragma once

#include <cinttypes>

class HrzrMetadataParser {
public:
   enum class TimeSyncSourceType { NO_TIME_SYNC = 0, GPS, GPS_NO_SYNC, NETWORK, NETWORK_NO_SYNC, UNKNOWN_TIME };

   enum class ClockSyncSourceType { INTERNAL_CLOCK = 0, EXTERNAL_CLOCK, UNKNOWN_CLOCK };

   static uint8_t getVersion(uint64_t metadata);
   static HrzrMetadataParser::TimeSyncSourceType getTimeSyncSource(uint64_t metadata);
   static HrzrMetadataParser::ClockSyncSourceType getClockSyncSource(uint64_t metadata);
};