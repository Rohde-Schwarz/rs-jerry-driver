#pragma once

#include "channelSettings.grpc.pb.h"
#include "channelSettings.pb.h"
#include "login.grpc.pb.h"
#include "login.pb.h"
#include "msr4RemoteStreaming.grpc.pb.h"
#include "msr4RemoteStreaming.pb.h"
#include "msr4RemoteStreamingFromMSR4.grpc.pb.h"
#include "msr4RemoteStreamingFromMSR4.pb.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

class GrpcClient {
public:
   enum class MSR4Channel { Rx1 = 50055, Rx2, Rx3, Rx4 };

   GrpcClient();

   grpc::Status MSR4Login();
   void SetCredentials(std::string name, std::string password);
   void SetIp(std::string ip);

#pragma region channelSettings
   RsIcpxGrpcService::ChannelSetting GetChannelSettings(MSR4Channel channel);
   RsIcpxGrpcService::ErrorMessage SetRfSetting(MSR4Channel channel, RsIcpxGrpcService::RFSetting rf_setting);
   RsIcpxGrpcService::ErrorMessage SetBandwidthSettingBySampleRate(MSR4Channel channel, RsIcpxGrpcService::SampleRateSetting sample_rate_setting);
   RsIcpxGrpcService::ErrorMessage SetBandwidthSettingByAnalysisBandwidth(MSR4Channel channel,
                                                                          RsIcpxGrpcService::AnalysisBandwidthSetting analysis_bandwidth_setting);
   RsIcpxGrpcService::ErrorMessage AutoLevel(MSR4Channel channel);
#pragma endregion channelSettings

#pragma region msr4RemoteStreaming
   RsIcpxGrpcService::NetworkStreamingSettings GetNetworkStreamingSettings(MSR4Channel msr4_channel);
   RsIcpxGrpcService::ErrorMessage SetNetworkStreamingSetting(MSR4Channel msr4_channel, RsIcpxGrpcService::NetworkStreamingSetting network_streaming_setting);
#pragma endregion msr4RemoteStreaming

#pragma region msr4RemoteStreamingFromMSR4
   RsIcpxGrpcService::UdpSetting GetUDPSetting(MSR4Channel msr4_channel);
   RsIcpxGrpcService::UdpStatus GetUDPStatus(MSR4Channel msr4_channel);
   RsIcpxGrpcService::ReadyMessage IsUDPStatusReady(MSR4Channel msr4_channel);
   RsIcpxGrpcService::ErrorMessage SetUDPSetting(MSR4Channel msr4_channel, RsIcpxGrpcService::UdpSetting udp_setting);
#pragma endregion msr4RemoteStreamingFromMSR4

private:
   RsIcpxGrpcService::Credentials creds;
   std::string ip;
   RsIcpxGrpcService::JsonWebToken token;

   std::unique_ptr<grpc::ClientContext> GenerateContext();

   template<typename T>
   std::unique_ptr<typename T::Stub> GenerateStub(MSR4Channel msr4_channel);
};