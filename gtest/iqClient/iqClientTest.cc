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

struct receiver_args{
   IqClient *iqClient;
   int num_stream;
   bool *should_quit;
   uint64_t *num_total_samples_recv;
};

void *receive_thread(void* arg){
   receiver_args *my_arg = (receiver_args*)arg;
   int samples_to_receive = 1830;
   std::complex<float> *payload = (std::complex<float>*)malloc(sizeof(std::complex<float>)*samples_to_receive);

   while(!(*my_arg->should_quit)){
      int recv = my_arg->iqClient->GetSamples(my_arg->num_stream, samples_to_receive, &payload[0]);
      *(my_arg->num_total_samples_recv) += recv;
   }

   free(payload);
   return NULL;
}

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
      DpdkSource::stream_attr *streams = (DpdkSource::stream_attr *) malloc(sizeof(DpdkSource::stream_attr) * 1);

      DpdkSource::stream_attr stream1;
      stream1.dpdk_port_id = 0;
      stream1.udp_rx_port  = 5000;
      stream1.l_cores      = std::pair<int, int>(0, 24);
      streams[0]           = stream1;

      iqClient->SetupDpdkSource(streams, 1);

      ConnectToMSR4();
      iqClient->SetRxChannel(GrpcClient::MSR4Channel::Rx1);
      iqClient->SetProtocol(RsIcpxGrpcService::Protocols::HRZR);
      iqClient->SetPort(stream1.udp_rx_port);

      iqClient->SetStreamingStatus(true);

      int itr = 0;
      while (itr < nsamples_ptr->size())
      {
         nsamples_ptr->at(itr) = iqClient->GetSamples(0, 3660, payload->at(itr).data());
         if (nsamples_ptr->at(itr) > 0)
            itr++;
      }

      iqClient->SetStreamingStatus(false);
      iqClient->TeardownDpdkSource();
   }
};

TEST_F(IqClientTest, AmmosException)
{
   EXPECT_THROW(iqClient->SetProtocol(RsIcpxGrpcService::Protocols::AMMOS), NotImplementedException);
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
   DpdkSource::stream_attr *streams = (DpdkSource::stream_attr *) malloc(sizeof(DpdkSource::stream_attr) * 1);

   DpdkSource::stream_attr stream1;
   stream1.dpdk_port_id = 0;
   stream1.udp_rx_port  = 5000;
   stream1.l_cores      = std::pair<int, int>(0, 24);
   streams[0]           = stream1;

   iqClient->SetMSR4ByJson("full/path/to/rs-jerry-driver/configFiles/example.json");

   int number_of_samples = 4096;
   
   iqClient->SetDumpMode(true);
   iqClient->SetBandwidthBySampleRate(10000000);
   iqClient->SetPortID(0);
   iqClient->SetNorm(10000);
   iqClient->SetupDpdkSource(streams, 1);

   std::complex<float> *output = (std::complex<float> *)malloc(sizeof(std::complex<float>)*number_of_samples);

   iqClient->SetStreamingStatus(true);
   while(!((DpdkSource*)iqClient->GetSource())->shouldQuit()) {
      iqClient->GetSamples(0, number_of_samples, output);
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

TEST_F(IqClientTest, SendPayload)
{
   iqClient->hrzr_udp_transmitter = new HrzrUdpTransmitter();
   iqClient->hrzr_udp_transmitter->setLocalSockaddr("192.168.30.1", 5000);
   iqClient->hrzr_udp_transmitter->setDestSockaddr("192.168.40.1", 5001);
   iqClient->hrzr_udp_transmitter->openSocketAndConnect();
   
   auto localSockAddr = iqClient->hrzr_udp_transmitter->getLocalSockAddr();
   EXPECT_EQ(localSockAddr.sin_family, AF_INET);
   EXPECT_EQ(localSockAddr.sin_port, 5000);

   auto destSockAddr = iqClient->hrzr_udp_transmitter->getDestSockAddr();
   EXPECT_EQ(destSockAddr.sin_family, AF_INET);
   EXPECT_EQ(destSockAddr.sin_port, htons(5001));

   int complex_f_per_packet = 366;

   std::complex<float> *samples = (std::complex<float> *)malloc(sizeof(std::complex<float>)*complex_f_per_packet);
   for(int i = 0; i < complex_f_per_packet; i++){
      samples[i] = std::complex<float>(i, i);
   }

   int num_packets_to_send = 0x1001;
   for(int i = 0; i < num_packets_to_send; i++)
      iqClient->SendPayload(samples, complex_f_per_packet);      
}

TEST_F(IqClientTest, GetSamplesOnTwoStreams){
   int time_duration_in_seconds = 10;
   int samplerate_in_hz = 20000000; // = 20MHz

   ConnectToMSR4();

   cpu_set_t cpuset;
   pthread_t thread;
   thread = pthread_self();
   CPU_ZERO(&cpuset);
   CPU_SET(1, &cpuset);
   pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);

   DpdkSource::stream_attr *streams = (DpdkSource::stream_attr*)malloc(sizeof(DpdkSource::stream_attr) * 2);

   DpdkSource::stream_attr stream1;
   stream1.dpdk_port_id = 0;
   stream1.udp_rx_port = 5000;
   stream1.l_cores = std::pair<int, int>(0, 24);
   streams[0] = stream1;
   iqClient->SetRxChannel(GrpcClient::MSR4Channel::Rx1);
   iqClient->SetBandwidthBySampleRate(samplerate_in_hz);
   iqClient->SetProtocol(RsIcpxGrpcService::Protocols::HRZR);
   iqClient->SetPort(stream1.udp_rx_port);
   iqClient->SetDestinationAddress("192.168.30.1");

   DpdkSource::stream_attr stream2;
   stream2.dpdk_port_id = 1;
   stream2.udp_rx_port = 5001;
   stream2.l_cores = std::pair<int, int>(2, 26);
   streams[1] = stream2;
   iqClient->SetRxChannel(GrpcClient::MSR4Channel::Rx2);
   iqClient->SetBandwidthBySampleRate(samplerate_in_hz);
   iqClient->SetProtocol(RsIcpxGrpcService::Protocols::HRZR);
   iqClient->SetPort(stream2.udp_rx_port);
   iqClient->SetDestinationAddress("192.168.40.1");

   iqClient->SetNorm(10000);
   iqClient->SetupDpdkSource(streams, 2);

   receiver_args arg1;
   bool should_quit1 = false;
   uint64_t total_samp_recv1 = 0;
   arg1.num_stream = 0;
   arg1.should_quit = &should_quit1;
   arg1.iqClient = iqClient;
   arg1.num_total_samples_recv = &total_samp_recv1;

   receiver_args arg2;
   bool should_quit2 = false;
   uint64_t total_samp_recv2 = 0;
   arg2.num_stream = 1;
   arg2.should_quit = &should_quit2;
   arg2.iqClient = iqClient;
   arg2.num_total_samples_recv = &total_samp_recv2;

   pthread_t thread1;
   cpu_set_t cpuset1;
   CPU_ZERO(&cpuset1);
   CPU_SET(3, &cpuset1);
   pthread_create(&thread1, NULL, receive_thread, (void*)&arg1);
   pthread_setaffinity_np(thread1, sizeof(cpuset1), &cpuset1);

   pthread_t thread2;
   cpu_set_t cpuset2;
   CPU_ZERO(&cpuset2);
   CPU_SET(5, &cpuset2);
   pthread_create(&thread2, NULL, receive_thread, (void*)&arg2);
   pthread_setaffinity_np(thread2, sizeof(cpuset2), &cpuset2);

   iqClient->SetStreamingStatus(true);
   iqClient->SetRxChannel(GrpcClient::MSR4Channel::Rx1);
   iqClient->SetStreamingStatus(true);

   std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
   std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
   while(std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() < time_duration_in_seconds){
      end = std::chrono::steady_clock::now();
   }

   should_quit1 = true;
   should_quit2 = true;
   pthread_join(thread1, NULL);
   pthread_join(thread2, NULL);

   iqClient->SetStreamingStatus(false);
   iqClient->SetRxChannel(GrpcClient::MSR4Channel::Rx2);
   iqClient->SetStreamingStatus(false);

   std::cout << "Stream 1 received " << total_samp_recv1 << " samples in " << time_duration_in_seconds << "s @ SampleRate: " << samplerate_in_hz << "Hz" << std::endl;
   std::cout << "Stream 2 received " << total_samp_recv2 << " samples in " << time_duration_in_seconds << "s @ SampleRate: " << samplerate_in_hz << "Hz" << std::endl;

   free(streams);
}
