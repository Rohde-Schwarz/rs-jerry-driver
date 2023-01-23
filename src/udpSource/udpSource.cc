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

#include "udpSource.h"

#include "hrzrHeaderParser.h"

UdpSource::UdpSource(float norm)
   : hrzr_parser(std::make_unique<HrzrParser>(norm))
{
   connect();
}

UdpSource::~UdpSource()
{
   disconnect();
}

void UdpSource::connect()
{
   printf("SERVER: %s\n", SERVER);
   int i = sizeof(si_other);

   if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
   {
      die("socket");
   }

   memset((char *) &si_other, 0, sizeof(si_other));
   si_other.sin_family = AF_INET;
   si_other.sin_port   = htons(PORT);

   if (inet_pton(AF_INET, SERVER, &si_other.sin_addr) == 0)
   {
      fprintf(stderr, "inet_aton() failed\n");
      exit(1);
   }

   int ret = bind(s, (struct sockaddr *) &si_other, (socklen_t) slen);

   if (ret == -1)
   {
      die("bind()");
   }
}

void UdpSource::disconnect()
{
   //close(s);
}

// TODO caching of unused symbols missing
int UdpSource::getSamples(int number_of_samples, std::complex<float> *samples)
{
   auto packets_to_dequeue     = ((number_of_samples) / definitions::SAMPLES_PER_PACKET) + 1;
   auto total_samples_received = 0;

   for (int packet_idx = 0; packet_idx < packets_to_dequeue; packet_idx++)
   {

      int ret = recv(s, buf, definitions::PACKET_SIZE, 0);

      if (ret == -1)
      {
         die("recvfrom()");
      }
      if (ret != definitions::PACKET_SIZE)
      {
         printf("skipped payload\n");
         continue;
      }

      uint64_t header = 0;
      memcpy(&header, buf, sizeof(uint64_t));

      auto control = HrzrHeaderParser::getControlFromHeader(header);

      if (control == HrzrHeaderParser::PacketType::DATA)
      {
         auto samples_received = hrzr_parser->parsePayloadSamples(packet_idx, (int16_t *) buf, samples);

         total_samples_received += samples_received;
      }
   }
   return total_samples_received;
}

void UdpSource::die(const char *s)
{
   perror(s);
   exit(1);
}

void UdpSource::setIp(std::string ip)
{
   this->ip = ip;
}

void UdpSource::setPort(int port)
{
   this->port = port;
}