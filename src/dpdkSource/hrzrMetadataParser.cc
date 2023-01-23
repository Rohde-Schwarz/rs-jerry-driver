/*
MIT License 
Copyright (c) 2022 Rohde & Schwarz INRADIOS GmbH 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
