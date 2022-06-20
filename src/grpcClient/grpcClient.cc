#include "grpcClient.h"

GrpcClient::GrpcClient()
{}

grpc::Status GrpcClient::MSR4Login()
{
   auto channel = grpc::CreateChannel(ip + ":50060", grpc::InsecureChannelCredentials());
   auto stub    = RsIcpxGrpcService::MSR4Login::NewStub(std::move(channel));

   grpc::ClientContext context;

   return stub->login(&context, creds, &token);
}

void GrpcClient::SetCredentials(std::string name, std::string password)
{
   creds.set_name(name);
   creds.set_password(password);
}

void GrpcClient::SetIp(std::string ip)
{
   this->ip = ip;
}

RsIcpxGrpcService::ChannelSetting GrpcClient::GetChannelSettings(MSR4Channel msr4_channel)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::Empty empty;
   RsIcpxGrpcService::ChannelSetting channel_setting;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteChannel>(msr4_channel);
   stub->getChannelSetting(context.get(), empty, &channel_setting);
   return channel_setting;
}

RsIcpxGrpcService::ErrorMessage GrpcClient::SetRfSetting(MSR4Channel msr4_channel, RsIcpxGrpcService::RFSetting rf_setting)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::ErrorMessage error_message;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteChannel>(msr4_channel);
   stub->setRFSetting(context.get(), rf_setting, &error_message);
   return error_message;
}

RsIcpxGrpcService::ErrorMessage GrpcClient::SetBandwidthSettingBySampleRate(MSR4Channel msr4_channel, RsIcpxGrpcService::SampleRateSetting sample_rate_setting)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::ErrorMessage error_message;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteChannel>(msr4_channel);
   stub->setBandwidthSettingBySampleRate(context.get(), sample_rate_setting, &error_message);
   return error_message;
}

RsIcpxGrpcService::ErrorMessage GrpcClient::SetBandwidthSettingByAnalysisBandwidth(MSR4Channel msr4_channel,
                                                                                   RsIcpxGrpcService::AnalysisBandwidthSetting analysis_bandwidth_setting)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::ErrorMessage error_message;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteChannel>(msr4_channel);
   stub->setBandwidthSettingByAnalysisBandwidth(context.get(), analysis_bandwidth_setting, &error_message);
   return error_message;
}

RsIcpxGrpcService::ErrorMessage GrpcClient::AutoLevel(MSR4Channel msr4_channel)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::Empty empty;
   RsIcpxGrpcService::ErrorMessage error_message;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteChannel>(msr4_channel);
   stub->autoLevel(context.get(), empty, &error_message);
   return error_message;
}

RsIcpxGrpcService::NetworkStreamingSettings GrpcClient::GetNetworkStreamingSettings(MSR4Channel msr4_channel)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::Empty empty;
   RsIcpxGrpcService::NetworkStreamingSettings network_streaming_settings;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteStreaming>(msr4_channel);
   stub->getNetworkStreamingSettings(context.get(), empty, &network_streaming_settings);
   return network_streaming_settings;
}

RsIcpxGrpcService::ErrorMessage GrpcClient::SetNetworkStreamingSetting(MSR4Channel msr4_channel,
                                                                       RsIcpxGrpcService::NetworkStreamingSetting network_streaming_setting)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::ErrorMessage error_message;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteStreaming>(msr4_channel);
   stub->setNetworkStreamingSettings(context.get(), network_streaming_setting, &error_message);
   return error_message;
}

RsIcpxGrpcService::UdpSetting GrpcClient::GetUDPSetting(MSR4Channel msr4_channel)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::Empty empty;
   RsIcpxGrpcService::UdpSetting udp_setting;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteStreamingFromMSR4>(msr4_channel);
   stub->getUDPSetting(context.get(), empty, &udp_setting);
   return udp_setting;
}

RsIcpxGrpcService::UdpStatus GrpcClient::GetUDPStatus(MSR4Channel msr4_channel)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::Empty empty;
   RsIcpxGrpcService::UdpStatus udp_status;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteStreamingFromMSR4>(msr4_channel);
   stub->getUDPStatus(context.get(), empty, &udp_status);
   return udp_status;
}

RsIcpxGrpcService::ReadyMessage GrpcClient::IsUDPStatusReady(MSR4Channel msr4_channel)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::Empty empty;
   RsIcpxGrpcService::ReadyMessage ready_message;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteStreamingFromMSR4>(msr4_channel);
   stub->isUDPStatusReady(context.get(), empty, &ready_message);
   return ready_message;
}

RsIcpxGrpcService::ErrorMessage GrpcClient::SetUDPSetting(MSR4Channel msr4_channel, RsIcpxGrpcService::UdpSetting udp_setting)
{
   auto context = GenerateContext();

   RsIcpxGrpcService::ErrorMessage error_message;

   auto stub = GenerateStub<RsIcpxGrpcService::MSR4RemoteStreamingFromMSR4>(msr4_channel);
   stub->setUDPSetting(context.get(), udp_setting, &error_message);
   return error_message;
}

std::unique_ptr<grpc::ClientContext> GrpcClient::GenerateContext()
{
   auto context = std::make_unique<grpc::ClientContext>();
   context->AddMetadata("authorization", token.token());
   return context;
}

template<typename T>
std::unique_ptr<typename T::Stub> GrpcClient::GenerateStub(MSR4Channel msr4_channel)
{
   auto channel                           = grpc::CreateChannel(ip + ":" + std::to_string((int) msr4_channel), grpc::InsecureChannelCredentials());
   std::unique_ptr<typename T::Stub> stub = T::NewStub(std::move(channel));
   return stub;
}