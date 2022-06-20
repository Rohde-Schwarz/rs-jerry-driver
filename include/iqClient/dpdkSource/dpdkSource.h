#pragma once

#ifndef INCLUDED_DPDK_SOURCE_H
#define INCLUDED_DPDK_SOURCE_H

#include "../linuxSource.h"
#include "hrzrHeaderParser.h"
#include "hrzrParser.h"
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_flow.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_net.h>
#include <rte_udp.h>

#include <complex>
#include <memory>

class DpdkSource : public ILinuxSource {

public:
   ~DpdkSource() override;
   DpdkSource(int dpdk_port_id, uint16_t udp_rx_port, float norm);
   int getSamples(int number_of_samples, std::complex<float> *samples) override;

   bool shouldQuit() const { return quit; };
   int getPortID() const { return dpdk_port_id; };
   uint16_t getUdpRxPort() const { return udp_rx_port; };
   struct rte_ring *getRteRing() { return ring; };
   struct rte_mempool *getPool() { return mbuf_pool; };

private:
   void setupPort();
   void setupDpdk();
   void assertLinkStatus();
   static int RXThread(DpdkSource *dpdkSource);

   void setupRing();
   void startRXThread();
   void stopRXThread();
   bool isValidPacketType(HrzrHeaderParser::PacketType type);

   std::unique_ptr<HrzrParser> hrzr_parser;

   int dpdk_port_id;
   uint16_t udp_rx_port;
   bool quit;

   struct udp_hdr {
      uint16_t src_port;
      uint16_t dst_port;
      uint16_t dgram_len;
      uint16_t dgram_cksum;
   } __attribute__((__packed__));

   struct hrzr_packet_all_headers {

      struct rte_ether_hdr ether_hdr;
      struct rte_ipv4_hdr ipv4_hdr;
      struct udp_hdr udp;
      uint64_t hrzr;
   } __attribute__((__packed__));

   struct rte_mempool *mbuf_pool;
   struct rte_ring *ring;
};

#endif /* INCLUDED_DPDK_SOURCE_H */
