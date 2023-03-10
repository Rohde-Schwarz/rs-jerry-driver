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

#include "dpdkSource/hrzrHeaderParser.h"

#include "common/exceptions.h"
#include <arpa/inet.h>

HrzrHeaderParser::PacketType HrzrHeaderParser::getControlFromHeader(uint64_t hrzr)
{
   auto control = (static_cast<uint8_t>(hrzr) & 0xD0) >> 4;
   switch (control)
   {
      case 0x00:
         return PacketType::DATA;
      case 0x01:
         return PacketType::DATA_END_OF_BURST;
      case 0x04:
         return PacketType::FLOW_CONTROL;
      case 0x08:
         return PacketType::COMMAND;
      case 0x0C:
         return PacketType::COMMAND_RESPONSE;
      case 0x0D:
         return PacketType::COMMAND_RESPONSE_ERROR;
      case 0x05:
         return PacketType::METADATA;
      default:
         throw InvalidHrzrHeader("Could not match control.");
   }
}

uint16_t HrzrHeaderParser::getSequenceNumberFromHeader(uint64_t hrzr)
{
   uint16_t squ_nr_raw = static_cast<uint16_t>(hrzr) & 0xFF0F;
   uint16_t squ_nr     = ntohs(squ_nr_raw);
   return squ_nr;
}