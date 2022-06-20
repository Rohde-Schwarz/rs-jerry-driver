#include "hrzrParser.h"

#include "common/definitions.h"
#include "common/exceptions.h"
#include "dpdkSource.h"

HrzrParser::HrzrParser(float norm)
   : norm(norm)
{
   if (norm == 0)
      throw InvalidValueError("Norm cannot be 0.");
}

int HrzrParser::parsePayloadSamples(int packet_idx, int16_t *payload, std::complex<float> *samples)
{
   int total_samples_received = 0;

   for (int sample_idx_packet = 0; sample_idx_packet < definitions::SAMPLES_PER_PACKET; sample_idx_packet++)
   {
      samples[packet_idx * definitions::SAMPLES_PER_PACKET + sample_idx_packet].real(payload[sample_idx_packet * 2] / norm);
      samples[packet_idx * definitions::SAMPLES_PER_PACKET + sample_idx_packet].imag(payload[sample_idx_packet * 2 + 1] / norm);
      total_samples_received++;
   }
   return total_samples_received;
}
