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
   iqClient->SetMSR4ByJson("path/to/example.json");

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

TEST_F(IqClientTest, DISABLED_GetSamples)
{
   iqClient->SetMSR4ByJson("full/path/to/rs-jerry-driver/configFiles/example.json");

   int number_of_samples       = 1514;
   std::complex<float> *output = (std::complex<float> *) malloc(sizeof(std::complex<float>) * number_of_samples);

   iqClient->SetPortID(0);
   iqClient->SetupDpdkSource();

   iqClient->SetStreamingStatus(true);

   int nsamples = 0;
   while (true) {
      nsamples = iqClient->GetSamples(number_of_samples, output);
   }

   free(output);

   iqClient->TeardownDpdkSource();
}

TEST_F(IqClientTest, DISABLED_CanWriteSamplesToFile)
{
   iqClient->SetupDpdkSource();

   int number_of_samples       = 10;
   std::complex<float> *output = (std::complex<float> *) malloc(sizeof(std::complex<float>) * number_of_samples);

   std::fstream fs("binarysamples", std::fstream::out | std::fstream::binary);

   int nsamples = 0;
   while (nsamples < 10) { nsamples = iqClient->GetSamples(number_of_samples, output); }

   for (int i = 0; i < number_of_samples; i++) { fs << output[i] << std::endl; }
   fs.close();

   free(output);

   iqClient->TeardownDpdkSource();
}
