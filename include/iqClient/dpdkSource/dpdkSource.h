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
#include <vector>

class DpdkSource : public ILinuxSource {

public:
   struct stream_attr {
      std::pair<int, int> l_cores;
      int dpdk_port_id;
      uint16_t udp_rx_port;
      struct rte_mempool *mbuf_pool;
      struct rte_ring *ring;
   };

   struct rx_thread_arg{
      DpdkSource* dpdk_source;
      int num_stream;
   };

   DpdkSource(stream_attr *streams, int num_streams, float norm, bool dump_mode = false);
   ~DpdkSource() override;

   int getSamples(int num_stream, int number_of_samples, std::complex<float> *samples) override;

   bool shouldQuit() const { return quit; };
   int getPortID(int num_stream) const { return streams[num_stream].dpdk_port_id; };
   uint16_t getUdpRxPort(int num_stream) const { return streams[num_stream].udp_rx_port; };
   struct rte_ring *getRteRing(int num_stream) { return streams[num_stream].ring; };
   struct rte_mempool *getPool(int num_stream) { return streams[num_stream].mbuf_pool; };

private:
   void setupDpdk();
   void setupEal(std::vector<const char *> argv);
   void checkAvailablePorts();
   void allocateMemPool();

   void setupPort(int num_stream);
   void configureRxConf(int num_stream, rte_eth_rxconf &rxq_conf);
   rte_eth_dev_info getDevInfo(int num_stream);
   rte_eth_conf getPortConf(int num_stream, rte_eth_dev_info &dev_info);
   void configurePort(int num_stream, rte_eth_conf &port_conf);
   void setupRxQueues(int num_stream, rte_eth_rxconf &rxq_conf);
   void enablePromiscuousMode(int num_stream);
   void startPort(int num_stream);
   void assertLinkStatus(int num_stream);

   static int RXThread(rx_thread_arg *rx_thread_arg);
   void stopRXThread();
   static int dumpThread(DpdkSource *dpdkSource, int num_stream);
   bool isValidPacketType(HrzrHeaderParser::PacketType type);

   std::unique_ptr<HrzrParser> hrzr_parser;

   stream_attr *streams;
   int num_streams;

   bool quit;
   uint64_t total_packets;
   uint64_t total_packets_lost;

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
};

#endif /* INCLUDED_DPDK_SOURCE_H */
