#include "hrzrMetadataParser.h"

uint8_t HrzrMetadataParser::getVersion(uint64_t metadata)
{
   uint8_t version = static_cast<uint8_t>(metadata) >> 4;
   return version;
}

HrzrMetadataParser::TimeSyncSourceType HrzrMetadataParser::getTimeSyncSource(uint64_t metadata)
{
   uint8_t time_sync_source_raw = static_cast<uint8_t>(metadata) & 0xD;

   switch (time_sync_source_raw)
   {
      case 0x0:
         return HrzrMetadataParser::TimeSyncSourceType::NO_TIME_SYNC;
      case 0x9:
         return HrzrMetadataParser::TimeSyncSourceType::GPS;
      case 0x8:
         return HrzrMetadataParser::TimeSyncSourceType::GPS_NO_SYNC;
      case 0x5:
         return HrzrMetadataParser::TimeSyncSourceType::NETWORK;
      case 0x4:
         return HrzrMetadataParser::TimeSyncSourceType::NETWORK_NO_SYNC;
      default:
         return HrzrMetadataParser::TimeSyncSourceType::UNKNOWN_TIME;
   }
}

HrzrMetadataParser::ClockSyncSourceType HrzrMetadataParser::getClockSyncSource(uint64_t metadata)
{
   uint8_t clock_sync_source_raw = static_cast<uint16_t>(metadata) >> 14;

   switch (clock_sync_source_raw)
   {
      case 0x0:
         return HrzrMetadataParser::ClockSyncSourceType::INTERNAL_CLOCK;
      case 0x2:
         return HrzrMetadataParser::ClockSyncSourceType::EXTERNAL_CLOCK;
      default:
         return HrzrMetadataParser::ClockSyncSourceType::UNKNOWN_CLOCK;
   }
}
