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

#include "iqClient.h"

IqClient::IqClient()
   : norm(1)
   , grpc_client(new GrpcClient())
   , dump_mode(false)
{}

grpc::Status IqClient::MSR4Login()
{
   return grpc_client->MSR4Login();
}

void IqClient::SetMSR4Credentials(std::string name, std::string password)
{
   grpc_client->SetCredentials(name, password);
}

void IqClient::SetMSR4Ip(std::string ip)
{
   grpc_client->SetIp(ip);
}

void IqClient::SetRxChannel(GrpcClient::MSR4Channel rx)
{
   msr4_channel = rx;
}

RsIcpxGrpcService::ErrorMessage IqClient::SetPort(int port)
{
   RsIcpxGrpcService::UdpSetting my_setting = grpc_client->GetUDPSetting(msr4_channel);
   my_setting.set_port(port);
   return grpc_client->SetUDPSetting(msr4_channel, my_setting);
}

RsIcpxGrpcService::ErrorMessage IqClient::SetDestinationAddress(std::string destination_address)
{
   RsIcpxGrpcService::UdpSetting my_setting = grpc_client->GetUDPSetting(msr4_channel);
   my_setting.set_destinationaddress(destination_address);
   return grpc_client->SetUDPSetting(msr4_channel, my_setting);
}

RsIcpxGrpcService::ErrorMessage IqClient::SetProtocol(RsIcpxGrpcService::Protocols my_protocol)
{
   if (my_protocol == RsIcpxGrpcService::Protocols::AMMOS)
      throw NotImplementedException("AMMOS Protocol not yet implemented.");

   RsIcpxGrpcService::UdpSetting my_setting = grpc_client->GetUDPSetting(msr4_channel);
   my_setting.set_protocol(my_protocol);
   return grpc_client->SetUDPSetting(msr4_channel, my_setting);
}

RsIcpxGrpcService::ErrorMessage IqClient::SetStreamingStatus(bool status)
{
   RsIcpxGrpcService::UdpSetting my_setting   = grpc_client->GetUDPSetting(msr4_channel);
   RsIcpxGrpcService::StreamBase *stream_base = new RsIcpxGrpcService::StreamBase();
   stream_base->set_isstreaming(status);
   my_setting.set_allocated_base(stream_base);
   return grpc_client->SetUDPSetting(msr4_channel, my_setting);
}

RsIcpxGrpcService::ErrorMessage IqClient::SetSatFrequencyHz(double sat_frequency_hz)
{
   RsIcpxGrpcService::ChannelSetting my_setting = grpc_client->GetChannelSettings(msr4_channel);

   RsIcpxGrpcService::RFSetting rf_setting;
   rf_setting.set_satfrequencyhz(sat_frequency_hz);
   rf_setting.set_downfrequencyhz(my_setting.rfsetting().downfrequencyhz());

   return grpc_client->SetRfSetting(msr4_channel, rf_setting);
}

RsIcpxGrpcService::ErrorMessage IqClient::SetDownFrequencyHz(double down_frequency_hz)
{
   RsIcpxGrpcService::ChannelSetting my_setting = grpc_client->GetChannelSettings(msr4_channel);

   RsIcpxGrpcService::RFSetting rf_setting;
   rf_setting.set_satfrequencyhz(my_setting.rfsetting().satfrequencyhz());
   rf_setting.set_downfrequencyhz(down_frequency_hz);

   return grpc_client->SetRfSetting(msr4_channel, rf_setting);
}

RsIcpxGrpcService::ErrorMessage IqClient::SetBandwidthBySampleRate(double sample_rate_hz)
{
   RsIcpxGrpcService::SampleRateSetting sample_rate_setting;
   sample_rate_setting.set_sampleratehz(sample_rate_hz);

   return grpc_client->SetBandwidthSettingBySampleRate(msr4_channel, sample_rate_setting);
}

RsIcpxGrpcService::ErrorMessage IqClient::SetBandwidthByAnalysisBandwidth(double analysis_bandwidth_hz)
{
   RsIcpxGrpcService::AnalysisBandwidthSetting analysis_bandwidth_setting;
   analysis_bandwidth_setting.set_analysisbandwidthhz(analysis_bandwidth_hz);

   return grpc_client->SetBandwidthSettingByAnalysisBandwidth(msr4_channel, analysis_bandwidth_setting);
}

int IqClient::GetPort()
{
   RsIcpxGrpcService::UdpSetting my_setting = grpc_client->GetUDPSetting(msr4_channel);
   return my_setting.port();
}

std::string IqClient::GetDestinationAddress()
{
   RsIcpxGrpcService::UdpSetting my_setting = grpc_client->GetUDPSetting(msr4_channel);
   return my_setting.destinationaddress();
}

RsIcpxGrpcService::Protocols IqClient::GetProtocol()
{
   RsIcpxGrpcService::UdpSetting my_setting = grpc_client->GetUDPSetting(msr4_channel);
   return my_setting.protocol();
}

bool IqClient::GetStreamingStatus()
{
   RsIcpxGrpcService::UdpSetting my_setting = grpc_client->GetUDPSetting(msr4_channel);
   return my_setting.base().isstreaming();
}

double IqClient::GetSatFrequencyHz()
{
   RsIcpxGrpcService::ChannelSetting my_setting = grpc_client->GetChannelSettings(msr4_channel);
   return my_setting.rfsetting().satfrequencyhz();
}

double IqClient::GetDownFrequencyHz()
{
   RsIcpxGrpcService::ChannelSetting my_setting = grpc_client->GetChannelSettings(msr4_channel);
   return my_setting.rfsetting().downfrequencyhz();
}

double IqClient::GetSampleRate()
{
   RsIcpxGrpcService::ChannelSetting my_setting = grpc_client->GetChannelSettings(msr4_channel);
   return my_setting.bandwidthsetting().sampleratehz();
}

double IqClient::GetAnalysisBandwidth()
{
   RsIcpxGrpcService::ChannelSetting my_setting = grpc_client->GetChannelSettings(msr4_channel);
   return my_setting.bandwidthsetting().analysisbandwidthhz();
}

void IqClient::SetMSR4ByJson(std::string path)
{
   Json::Value root                       = JsonParser::GetRoot(path);
   JsonParser::LoginStruct login          = JsonParser::GetLoginStruct(root);
   JsonParser::RxChannelStruct rx_channel = JsonParser::GetRxChannelStruct(root);
   JsonParser::StreamingStruct stream     = JsonParser::GetStreamStruct(root);
   JsonParser::ChannelStruct channel      = JsonParser::GetChannelStruct(root);

   SetMSR4Credentials(login.user, login.password);
   SetMSR4Ip(login.ip);
   grpc::Status status = MSR4Login();
   if (!status.ok())
      throw InvalidValueError("Invalid login.");

   switch (rx_channel.rx_channel)
   {
      case 1:
         SetRxChannel(GrpcClient::MSR4Channel::Rx1);
         break;
      case 2:
         SetRxChannel(GrpcClient::MSR4Channel::Rx2);
         break;
      case 3:
         SetRxChannel(GrpcClient::MSR4Channel::Rx3);
         break;
      case 4:
         SetRxChannel(GrpcClient::MSR4Channel::Rx4);
         break;
      default:
         throw NotImplementedException("Invalid channel. RxChannel only accepts \"[1-4]\"");
         break;
   }
   RsIcpxGrpcService::ErrorMessage err;

   if (GetStreamingStatus())
      SetStreamingStatus(false);

   err = SetSatFrequencyHz(stream.sat_frequency);
   if (err.errorcode() != 0)
      throw InvalidValueError(err.errormessage());

   err = SetDownFrequencyHz(stream.down_frequency);
   if (err.errorcode() != 0)
      throw InvalidValueError(err.errormessage());

   err = SetBandwidthByAnalysisBandwidth(stream.bandwith_by_analysis_bandwidth);
   if (err.errorcode() != 0)
      throw InvalidValueError(err.errormessage());

   err = SetPort(channel.port);
   if (err.errorcode() != 0)
      throw InvalidValueError(err.errormessage());

   err = SetDestinationAddress(channel.destination_address);
   if (err.errorcode() != 0)
      throw InvalidValueError(err.errormessage());

   if (channel.protocol == "HRZR")
      SetProtocol(RsIcpxGrpcService::Protocols::HRZR);
   else
      throw NotImplementedException("AMMOS Protocol not yet implemented.");
}

void IqClient::SetPortID(int port_id)
{
   this->port_id = port_id;
}

void IqClient::SetNorm(float norm)
{
   this->norm = norm;
}

void IqClient::SetupDpdkSource(DpdkSource::stream_attr *streams, int num_streams)
{
   sample_source = new DpdkSource(streams, num_streams, norm, dump_mode);
}

void IqClient::TeardownDpdkSource()
{
   delete sample_source;
}

void IqClient::SetupUdpSource()
{
   sample_source = new UdpSource(norm);
}

void IqClient::TeardownUdpSource()
{
   delete sample_source;
}

int IqClient::GetSamples(int num_stream, int number_of_samples, std::complex<float> *samples)
{
   return sample_source->getSamples(num_stream, number_of_samples, samples);
}

void IqClient::SendPayload(std::complex<float> *samples, int number_of_samples){
   std::vector<int16_t> smpl(number_of_samples*2);
   for(int i = 0; i < number_of_samples; i++){
      smpl[2*i] = samples[i].real();
      smpl[2*i+1] = samples[i].imag();
   }
   hrzr_udp_transmitter->sendPacket(smpl);
}