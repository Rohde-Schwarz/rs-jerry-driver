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
