#pragma once
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string>
#include <vector>
#include "../common/definitions.h"

class HrzrUdpTransmitter {
public:
  HrzrUdpTransmitter();

  void setLocalSockaddr(std::string ip, uint16_t port);
  void setDestSockaddr(std::string ip, uint16_t port);
  void openSocketAndConnect();

  void sendPacket(std::vector<int16_t> samples);

  sockaddr_in getLocalSockAddr(){return local;}
  sockaddr_in getDestSockAddr(){return dest;}
  uint16_t getCurrentSequenceNumber(){return current_sequence_number;}

private:
  struct HrzrHeader{
    uint8_t control;
    uint16_t sequence_number;
    uint16_t total_packt_length;
    uint32_t source_id;
  };

  struct HrzrPayload{
    std::vector<int16_t> samples;
  };

  struct HrzrPacket{
    uint16_t ctrl_and_seq_nb;
    uint16_t total_pck_len;
    uint32_t src_id;
    int16_t samples[definitions::SAMPLES_PER_PACKET];
  }__attribute__( ( packed ) );

  struct sockaddr_in local;
  struct sockaddr_in dest;
  int my_sock;
  uint16_t current_sequence_number;
  
  HrzrHeader makeHrzrHeader(int num_samples);
  HrzrPayload makeHrzrPayload(std::vector<int16_t> samples);
  HrzrPacket makeHrzrPacket(HrzrHeader header, HrzrPayload payload);
};