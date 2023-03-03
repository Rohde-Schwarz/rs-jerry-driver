#include "../../include/iqClient/udpTransmitter/hrzrUdpTransmitter.h"
#include "../../include/iqClient/common/definitions.h"
#include <arpa/inet.h>

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

void HrzrUdpTransmitter::sendPacket(int16_t *samples, int num_samples)
{
    HrzrHeader header = makeHrzrHeader(num_samples);
    HrzrPayload payload = makeHrzrPayload(samples, num_samples);
    HrzrPacket packet = makeHrzrPacket(header, payload);
    sendto(my_sock, (const void*)&packet, packet.total_pck_len, 0, (sockaddr *)&dest, sizeof(dest));
    current_sequence_number++;
    if(current_sequence_number == 4096)
        current_sequence_number = 0;
}

HrzrUdpTransmitter::HrzrHeader HrzrUdpTransmitter::makeHrzrHeader(int num_samples){
    HrzrHeader header;
    header.control = 0;
    header.sequence_number = current_sequence_number;
    header.source_id = 0;
    header.total_packt_length = 8 + num_samples * definitions::SAMPLE_SIZE_IN_BYTE; // 8 byte hrzr; see definitions.h
    return header;
}

HrzrUdpTransmitter::HrzrPayload HrzrUdpTransmitter::makeHrzrPayload(int16_t *samples, int num_samples){
    HrzrPayload payload;
    payload.num_samples = num_samples;
    payload.samples = (int16_t*)malloc(sizeof(int16_t)*num_samples);
    for(int i = 0; i < num_samples; i++){
        payload.samples[i] = samples[i];
    }
    return payload;
}

HrzrUdpTransmitter::HrzrPacket HrzrUdpTransmitter::makeHrzrPacket(HrzrHeader header, HrzrPayload payload){
    HrzrPacket packet;
    packet.ctrl_and_seq_nb = (static_cast<uint16_t>(header.control) << 12) | header.sequence_number;
    packet.total_pck_len = header.total_packt_length;
    packet.src_id = header.source_id;
    packet.samples = payload.samples;

    return packet;
}