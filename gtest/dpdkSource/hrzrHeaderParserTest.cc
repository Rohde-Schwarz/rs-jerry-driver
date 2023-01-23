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
