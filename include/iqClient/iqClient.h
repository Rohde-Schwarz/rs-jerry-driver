#pragma once

#ifndef INCLUDED_IQ_CLIENT_H
#define INCLUDED_IQ_CLIENT_H

#include "common/exceptions.h"
#include "common/jsonParser.h"
#include "dpdkSource/dpdkSource.h"
#include "grpcClient.h"
#include "udpSource/udpSource.h"
#include "udpTransmitter/hrzrUdpTransmitter.h"

class IqClient {
public:
   IqClient();
   HrzrUdpTransmitter *hrzr_udp_transmitter;

#pragma region MSR4
   grpc::Status MSR4Login();
   void SetMSR4Credentials(std::string name, std::string password);
   void SetMSR4Ip(std::string ip);

   void SetRxChannel(GrpcClient::MSR4Channel rx);

   RsIcpxGrpcService::ErrorMessage SetPort(int port);
   RsIcpxGrpcService::ErrorMessage SetDestinationAddress(std::string destination_address);
   RsIcpxGrpcService::ErrorMessage SetProtocol(RsIcpxGrpcService::Protocols protocol);
   RsIcpxGrpcService::ErrorMessage SetStreamingStatus(bool status);

   RsIcpxGrpcService::ErrorMessage SetSatFrequencyHz(double sat_frequency_hz);
   RsIcpxGrpcService::ErrorMessage SetDownFrequencyHz(double down_frequency_hz);
   RsIcpxGrpcService::ErrorMessage SetBandwidthBySampleRate(double sample_rate_hz);
   RsIcpxGrpcService::ErrorMessage SetBandwidthByAnalysisBandwidth(double analysis_bandwidth_hz);

   GrpcClient::MSR4Channel GetRxChannel()
   {
      return msr4_channel;
   }

   int GetPort();
   std::string GetDestinationAddress();
   RsIcpxGrpcService::Protocols GetProtocol();
   bool GetStreamingStatus();

   double GetSatFrequencyHz();
   double GetDownFrequencyHz();
   double GetSampleRate();
   double GetAnalysisBandwidth();

   void SetMSR4ByJson(std::string path);

   std::string GetVersion()
   {
      return VERSION;
   };

#pragma endregion MSR4

   // WARNING
   // For testing purpose only - REALLY bad performance at times
   // Dumps 3seconds of hex-encoded data to a plain text file(size will be ~180M)
   void SetDumpMode(bool value){
      dump_mode = value;
   }
   bool GetDumpMode()
   {
      return dump_mode;
   }

   void SetPortID(int port_id);
   int GetPortID()
   {
      return port_id;
   };

   void SetNorm(float norm);
   float GetNorm()
   {
      return norm;
   }

   ILinuxSource *GetSource()
   {
      return sample_source;
   }

   void SetupDpdkSource(DpdkSource::stream_attr *streams, int num_streams);
   void TeardownDpdkSource();

   void SetupUdpSource();
   void TeardownUdpSource();

   int GetSamples(int num_stream, int number_of_samples, std::complex<float> *samples);
   void SendPayload(std::complex<float> *samples, int number_of_samples);

private:
   const std::string VERSION = "1.0";

   GrpcClient::MSR4Channel msr4_channel;

   int port_id;
   bool dump_mode;
   float norm;

   ILinuxSource *sample_source;
   GrpcClient *grpc_client;
};

#endif /* INCLUDED_IQ_CLIENT_H */