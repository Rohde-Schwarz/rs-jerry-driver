#pragma once

#include <jsoncpp/json/json.h>

#include <fstream>
#include <iostream>

class JsonParser {
public:
   struct LoginStruct {
      std::string ip;
      std::string user;
      std::string password;
   };
   struct RxChannelStruct {
      int rx_channel;
   };
   struct StreamingStruct {
      int sat_frequency;
      int down_frequency;
      int bandwith_by_analysis_bandwidth;
   };
   struct ChannelStruct {
      int port;
      std::string destination_address;
      std::string protocol;
   };

   static Json::Value GetRoot(const std::string &path)
   {
      Json::Value root;
      std::ifstream ifs;
      ifs.open(path);

      if (!ifs.is_open())
      {
         std::cout << "Could not open json file" << std::endl;
         return root;
      }

      Json::CharReaderBuilder builder;
      JSONCPP_STRING errs;
      if (!parseFromStream(builder, ifs, &root, &errs))
      {
         std::cout << errs << std::endl;
      }

      return root;
   }

   static LoginStruct GetLoginStruct(Json::Value root)
   {
      LoginStruct login;
      if (!root.isMember("LoginSettings"))
         return login;

      Json::Value empty;
      Json::Value loginSettings = root.get("LoginSettings", empty);

      login.ip       = loginSettings.get("Ip", empty).asString();
      login.user     = loginSettings.get("User", empty).asString();
      login.password = loginSettings.get("Password", empty).asString();
      return login;
   }

   static RxChannelStruct GetRxChannelStruct(Json::Value root)
   {
      RxChannelStruct rx_channel;
      if (!root.isMember("RxChannelSettings"))
         return rx_channel;

      Json::Value empty;
      Json::Value rx_channel_settings = root.get("RxChannelSettings", empty);

      rx_channel.rx_channel = rx_channel_settings.get("RxChannel", empty).asInt();
      return rx_channel;
   }

   static StreamingStruct GetStreamStruct(Json::Value root)
   {
      StreamingStruct stream;
      if (!root.isMember("StreamingSettings"))
         return stream;

      Json::Value empty;
      Json::Value streamingSettings = root.get("StreamingSettings", empty);

      stream.sat_frequency                  = streamingSettings.get("SatFrequency", empty).asInt();
      stream.down_frequency                 = streamingSettings.get("DownFrequency", empty).asInt();
      stream.bandwith_by_analysis_bandwidth = streamingSettings.get("BandwidthByAnalysisBandwidth", empty).asInt();
      return stream;
   }

   static ChannelStruct GetChannelStruct(Json::Value root)
   {
      ChannelStruct channel;
      if (!root.isMember("ChannelSettings"))
         return channel;

      Json::Value empty;
      Json::Value channelSettings = root.get("ChannelSettings", empty);

      channel.port                = channelSettings.get("Port", empty).asInt();
      channel.destination_address = channelSettings.get("DestinationAddress", empty).asString();
      channel.protocol            = channelSettings.get("Protocol", empty).asString();
      return channel;
   }
};