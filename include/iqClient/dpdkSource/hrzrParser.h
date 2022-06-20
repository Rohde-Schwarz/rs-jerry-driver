#pragma once

#include <complex>

class HrzrParser {
public:
   enum class PacketType {
      DATA = 0,
      DATA_END_OF_BURST,
      FLOW_CONTROL,
      COMMAND,
      COMMAND_RESPONSE,
      COMMAND_RESPONSE_ERROR,
      METADATA,
   };

   HrzrParser(float norm);
   int parsePayloadSamples(int packet_idx, int16_t *payload, std::complex<float> *samples);

private:
   float norm;
};