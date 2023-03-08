#include "../../include/iqClient/udpTransmitter/hrzrUdpTransmitter.h"

#include <arpa/inet.h>
#include <iostream>
#include <cstring>

HrzrUdpTransmitter::HrzrUdpTransmitter():
current_sequence_number(0)
{}

void HrzrUdpTransmitter::setLocalSockaddr(std::string ip, uint16_t port){
    local.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &local.sin_addr.s_addr);
    local.sin_port = port;
}

void HrzrUdpTransmitter::setDestSockaddr(std::string ip, uint16_t port){
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &dest.sin_addr.s_addr);
    dest.sin_port = htons(port);
}

void HrzrUdpTransmitter::openSocketAndConnect()
{
    my_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bind(my_sock, (sockaddr *)&local, sizeof(local) );
    connect(my_sock, (struct sockaddr*) &dest, sizeof( dest ) );
}

void HrzrUdpTransmitter::sendPacket(std::vector<int16_t> samples)
{
    HrzrHeader header = makeHrzrHeader(samples.size());
    HrzrPayload payload = makeHrzrPayload(samples);
    HrzrPacket packet = makeHrzrPacket(header, payload);
    send(my_sock, (const void*)&packet, packet.total_pck_len, 0);
    current_sequence_number++;
    if(current_sequence_number == 4096)
        current_sequence_number = 0;
}

HrzrUdpTransmitter::HrzrHeader HrzrUdpTransmitter::makeHrzrHeader(int num_samples){
    HrzrHeader header;
    header.control = 0;
    header.sequence_number = current_sequence_number;
    header.source_id = 0;
    header.total_packt_length = 8 + num_samples * definitions::SAMPLE_SIZE_IN_BYTE * 2; // 8 byte hrzr; see definitions.h
    return header;
}

HrzrUdpTransmitter::HrzrPayload HrzrUdpTransmitter::makeHrzrPayload(std::vector<int16_t> samples){
    HrzrPayload payload;
    payload.samples = samples;
    return payload;
}

HrzrUdpTransmitter::HrzrPacket HrzrUdpTransmitter::makeHrzrPacket(HrzrHeader header, HrzrPayload payload){
    HrzrPacket packet;
    packet.ctrl_and_seq_nb = (static_cast<uint16_t>(header.control) << 12) | header.sequence_number;
    packet.total_pck_len = header.total_packt_length;
    packet.src_id = header.source_id;
    packet.samples[payload.samples.size()];
    std::memcpy(packet.samples, payload.samples.data(), sizeof(int16_t)*payload.samples.size());
    return packet;
}