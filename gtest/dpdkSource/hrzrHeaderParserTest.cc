#include "common/hrzrTestBinaryHelper.h"
#include "dpdkSource/hrzrHeaderParser.h"

#include <gtest/gtest.h>

#include <fstream>

static const auto HrzrPacketTypes = std::array<HrzrHeaderParser::PacketType, 7>({HrzrHeaderParser::PacketType::DATA,
                                                                                 HrzrHeaderParser::PacketType::DATA_END_OF_BURST,
                                                                                 HrzrHeaderParser::PacketType::FLOW_CONTROL,
                                                                                 HrzrHeaderParser::PacketType::COMMAND,
                                                                                 HrzrHeaderParser::PacketType::COMMAND_RESPONSE,
                                                                                 HrzrHeaderParser::PacketType::COMMAND_RESPONSE_ERROR,
                                                                                 HrzrHeaderParser::PacketType::METADATA});

TEST(HrzrHeaderParserTest, parsePacketFlags)
{
   HrzrHeaderParser headerParser;
   const auto testInput = HrzrTestBinaryHelper::fillInputFromFile("controltypes.bin");
   for (size_t i = 0; i < HrzrPacketTypes.size(); i++) EXPECT_EQ(HrzrPacketTypes[i], headerParser.getControlFromHeader(testInput.at(i)));
}

TEST(HrzrHeaderParserTest, parsePacketFlagsWithFractionalTimestamp)
{
   HrzrHeaderParser headerParser;
   const auto testInput = HrzrTestBinaryHelper::fillInputFromFile("controltypesWithTimestamp.bin");
   for (size_t i = 0; i < HrzrPacketTypes.size(); i++) EXPECT_EQ(HrzrPacketTypes[i], headerParser.getControlFromHeader(testInput.at(i)));
}

TEST(HrzrHeaderParserTest, parseSequenceNumber)
{
   HrzrHeaderParser headerParser;
   const auto testInput = HrzrTestBinaryHelper::fillInputFromFile("controltypes.bin");
   EXPECT_EQ(HrzrPacketTypes.size(), testInput.size());
   for (size_t i = 0; i < testInput.size(); i++) EXPECT_EQ(i, headerParser.getSequenceNumberFromHeader(testInput.at(i)));
}

TEST(HrzrHeaderParserTest, parseReverseSequenceNumber)
{
   HrzrHeaderParser headerParser;
   const auto testInput = HrzrTestBinaryHelper::fillInputFromFile("controltypesReverseSequenceNumber.bin");
   EXPECT_EQ(HrzrPacketTypes.size(), testInput.size());
   for (size_t i = 0; i < testInput.size(); i++) EXPECT_EQ(4095U - i, headerParser.getSequenceNumberFromHeader(testInput.at(i)));
}
