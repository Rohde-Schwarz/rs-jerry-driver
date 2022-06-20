#include "common/hrzrTestBinaryHelper.h"
#include "dpdkSource/hrzrMetadataParser.h"

#include <gtest/gtest.h>

#include <fstream>

static const auto HrzrMetadataTimeSyncSourceTypes =
    std::array<HrzrMetadataParser::TimeSyncSourceType, 6>({HrzrMetadataParser::TimeSyncSourceType::NO_TIME_SYNC,
                                                           HrzrMetadataParser::TimeSyncSourceType::GPS,
                                                           HrzrMetadataParser::TimeSyncSourceType::GPS_NO_SYNC,
                                                           HrzrMetadataParser::TimeSyncSourceType::NETWORK,
                                                           HrzrMetadataParser::TimeSyncSourceType::NETWORK_NO_SYNC,
                                                           HrzrMetadataParser::TimeSyncSourceType::UNKNOWN_TIME});

static const auto HrzrMetadataClockSyncSourceTypes =
    std::array<HrzrMetadataParser::ClockSyncSourceType, 3>({HrzrMetadataParser::ClockSyncSourceType::INTERNAL_CLOCK,
                                                            HrzrMetadataParser::ClockSyncSourceType::EXTERNAL_CLOCK,
                                                            HrzrMetadataParser::ClockSyncSourceType::UNKNOWN_CLOCK});

TEST(HrzrMetadataTest, parseVersion)
{
   HrzrMetadataParser metadata_parser;
   const auto test_input = HrzrTestBinaryHelper::fillInputFromFile("metadata_time_clock_types.bin");
   EXPECT_EQ(0, metadata_parser.getVersion(test_input.at(0)));
}

TEST(HrzrMetadataTest, parseTimeSyncSource)
{
   HrzrMetadataParser metadata_parser;
   const auto test_input = HrzrTestBinaryHelper::fillInputFromFile("metadata_time_clock_types.bin");
   for (size_t i = 0; i < HrzrMetadataTimeSyncSourceTypes.size(); i++)
      EXPECT_EQ(HrzrMetadataTimeSyncSourceTypes[i], metadata_parser.getTimeSyncSource(test_input.at(i)));
}

TEST(HrzrMetadataTest, parseClockSyncSource)
{
   HrzrMetadataParser metadata_parser;
   const auto test_input = HrzrTestBinaryHelper::fillInputFromFile("metadata_time_clock_types.bin");
   for (size_t i = 0; i < HrzrMetadataClockSyncSourceTypes.size(); i++)
      EXPECT_EQ(HrzrMetadataClockSyncSourceTypes[i], metadata_parser.getClockSyncSource(test_input.at(i)));
}
