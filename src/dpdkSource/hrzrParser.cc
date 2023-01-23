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
