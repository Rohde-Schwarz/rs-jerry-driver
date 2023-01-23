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

#include "iqClient/grpcClient.h"
#include "iqClient/iqClient.h"

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>

class IqClientTest : public ::testing::Test {
protected:
   IqClient *iqClient;
   void SetUp() override { iqClient = new IqClient(); }

   grpc::Status ConnectToMSR4()
   {
      iqClient->SetMSR4Ip("some.device.net");
      iqClient->SetMSR4Credentials("user", "password");
      return iqClient->MSR4Login();
   }

   void ReceivePackets(std::vector<std::vector<std::complex<float>>> *payload, std::vector<int> *nsamples_ptr)
   {
      iqClient->SetMSR4ByJson("full/path/to/rs-jerry-driver/configFiles/example.json");

      iqClient->SetPortID(0);
      iqClient->SetNorm(10000);
      iqClient->SetupDpdkSource();

      iqClient->SetStreamingStatus(true);

      int itr = 0;
      while (itr < nsamples_ptr->size())
      {
         nsamples_ptr->at(itr) = iqClient->GetSamples(3660, payload->at(itr).data());
         if (nsamples_ptr->at(itr) > 0)
            itr++;
      }

      iqClient->SetStreamingStatus(false);
      iqClient->TeardownDpdkSource();
   }
};

TEST_F(IqClientTest, AmmosException)
{
   iqClient->SetProtocol(RsIcpxGrpcService::Protocols::AMMOS);
   EXPECT_THROW(iqClient->SetupDpdkSource(), NotImplementedException);
}

TEST_F(IqClientTest, CanLogIntoMSR4)
{
   grpc::Status status = ConnectToMSR4();

   EXPECT_TRUE(status.ok());
   EXPECT_EQ(status.error_code(), 0);
}

TEST_F(IqClientTest, CanSetStreamingSettings)
{
   ConnectToMSR4();

   iqClient->SetRxChannel(GrpcClient::MSR4Channel::Rx1);
   iqClient->SetPort(12);
   iqClient->SetDestinationAddress("192.168.30.1");
   iqClient->SetProtocol(RsIcpxGrpcService::Protocols::HRZR);

   EXPECT_EQ(iqClient->GetRxChannel(), GrpcClient::MSR4Channel::Rx1);
   EXPECT_EQ(iqClient->GetPort(), 12);
   EXPECT_EQ(iqClient->GetDestinationAddress(), "192.168.30.1");
   EXPECT_EQ(iqClient->GetProtocol(), RsIcpxGrpcService::Protocols::HRZR);
}

TEST_F(IqClientTest, CanSetChannelSettings)
{
   ConnectToMSR4();

   iqClient->SetRxChannel(GrpcClient::MSR4Channel::Rx1);
   iqClient->SetSatFrequencyHz(1600000000);
   iqClient->SetDownFrequencyHz(1000000);
   iqClient->SetBandwidthByAnalysisBandwidth(195000000);

   EXPECT_EQ(iqClient->GetSatFrequencyHz(), 1600000000);
   EXPECT_EQ(iqClient->GetDownFrequencyHz(), 1000000);
   EXPECT_EQ(iqClient->GetAnalysisBandwidth(), 195000000);
}

TEST_F(IqClientTest, CanSetMSR4ByJson)
{
   iqClient->SetMSR4ByJson("full/path/to/rs-jerry-driver/configFiles/example.json");

   EXPECT_EQ(iqClient->GetPort(), 5001);
   EXPECT_EQ(iqClient->GetDestinationAddress(), "127.0.0.1");
   EXPECT_EQ(iqClient->GetProtocol(), RsIcpxGrpcService::Protocols::HRZR);
   EXPECT_EQ(iqClient->GetSatFrequencyHz(), 1500000000);
   EXPECT_EQ(iqClient->GetDownFrequencyHz(), 0);
   EXPECT_EQ(iqClient->GetAnalysisBandwidth(), 194100000);
}

TEST_F(IqClientTest, CanSetDPDKSettings)
{
   iqClient->SetPortID(0);
   iqClient->SetNorm(10);

   EXPECT_EQ(iqClient->GetPortID(), 0);
   EXPECT_EQ(iqClient->GetNorm(), 10);
}

TEST_F(IqClientTest, DISABLED_DumpHrzr){
   iqClient->SetMSR4ByJson("full/path/to/rs-jerry-driver/configFiles/example.json");

   int number_of_samples = 4096;
   
   iqClient->SetDumpMode(true);
   iqClient->SetBandwidthBySampleRate(10000000);
   iqClient->SetPortID(0);
   iqClient->SetNorm(10000);
   iqClient->SetupDpdkSource();

   std::complex<float> *output = (std::complex<float> *)malloc(sizeof(std::complex<float>)*number_of_samples);

   iqClient->SetStreamingStatus(true);
   while(!((DpdkSource*)iqClient->GetSource())->shouldQuit()) {
      iqClient->GetSamples(number_of_samples, output);
   }
   iqClient->SetStreamingStatus(false);

   free(output);
   iqClient->TeardownDpdkSource();
}

TEST_F(IqClientTest, DISABLED_GetSamples)
{
   int number_of_packets = 100;

   std::vector<std::vector<std::complex<float>>> payload(number_of_packets, std::vector<std::complex<float>>(4032));
   std::vector<int>payload_size(number_of_packets);

   ReceivePackets(&payload, &payload_size);
}

TEST_F(IqClientTest, WriteSamplesToFile)
{
   int number_of_packets = 100;

   std::vector<std::vector<std::complex<float>>> payload(number_of_packets, std::vector<std::complex<float>>(4032));
   std::vector<int> payload_size(number_of_packets);

   ReceivePackets(&payload, &payload_size);

   std::ofstream fs("binarysamples.bin", std::fstream::binary);
   for (int i = 0; i < number_of_packets; i++)
   {
      for (int j = 0; j < payload_size[i]; j++) {
         fs.write(reinterpret_cast<const char *>(&payload[i][j]), sizeof(std::complex<float>));
      }
   }
}
