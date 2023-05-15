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

#include "dpdkSource.h"

#include "common/definitions.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <thread>

DpdkSource::DpdkSource(stream_attr *streams, int num_streams, float norm, bool dump_mode)
   : hrzr_parser(std::make_unique<HrzrParser>(norm))
   , streams(streams)
   , num_streams(num_streams)
   , quit(false)
   , total_packets(0)
   , total_packets_lost(0)
{
   setupDpdk();

   for (int i = 0; i < num_streams; i++)
   {
      setupPort(i);

      std::string ring_name("rx_ring_" + std::to_string(i));
      streams[i].ring = rte_ring_create(ring_name.c_str(), definitions::RING_SIZE, rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
   }

   // Note: for some reason we need to set up all ports, mempools and rings before launching the first RXThread
   for (int i = 0; i < num_streams; i++)
   {
      auto *rx_arg        = (rx_thread_arg *) malloc(sizeof(rx_thread_arg));
      rx_arg->dpdk_source = this;
      rx_arg->num_stream  = i;

      if (dump_mode)
      {
         rte_eal_remote_launch((lcore_function_t *) dumpThread, (void *) rx_arg, streams[i].l_cores.second);
      }
      else
      {
         rte_eal_remote_launch((lcore_function_t *) RXThread, (void *) rx_arg, streams[i].l_cores.second);
      }
   }
}

void DpdkSource::setupDpdk()
{
   // For most argv params, please see: https://doc.dpdk.org/guides/linux_gsg/linux_eal_parameters.html
   std::string l_cores_string("");
   for (int i = 0; i < num_streams; i++)
   {
      l_cores_string += std::to_string(streams[i].l_cores.first) + "," + std::to_string(streams[i].l_cores.second);
      if (i + 1 < num_streams)
         l_cores_string += ",";
   }

   std::vector<const char *> argv{"dpdk", "-l", l_cores_string.c_str(), "--log-level", "lib.eal:error"};
   setupEal(std::move(argv));
   checkAvailablePorts();
   allocateMemPool();
}

void DpdkSource::setupEal(std::vector<const char *> argv)
{
   int ret = rte_eal_init(argv.size(), (char **) argv.data());
   if (ret < 0)
      rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
}

void DpdkSource::checkAvailablePorts()
{
   uint16_t nr_ports = rte_eth_dev_count_avail();
   if (nr_ports == 0)
      rte_exit(EXIT_FAILURE, "No Ethernet ports found\n");

   if (nr_ports != num_streams)
   {
      printf("Warn: %d VF ports detected, but we use %u\n", nr_ports, num_streams);
   }
}

void DpdkSource::allocateMemPool()
{
   for (int i = 0; i < num_streams; i++)
   {
      std::string mbuf_pool_name("mbuf_pool_" + std::to_string(i));
      streams[i].mbuf_pool = rte_pktmbuf_pool_create(mbuf_pool_name.c_str(), 4096, 0, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
      if (streams[i].mbuf_pool == NULL)
         rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");
   }
}

void DpdkSource::setupPort(int num_stream)
{
   struct rte_eth_rxconf rxq_conf;
   configureRxConf(num_stream, rxq_conf);

   setupRxQueues(num_stream, rxq_conf);
   enablePromiscuousMode(num_stream);

   startPort(num_stream);
   assertLinkStatus(num_stream);
}

void DpdkSource::configureRxConf(int num_stream, rte_eth_rxconf &rxq_conf)
{
   rte_eth_dev_info dev_info = getDevInfo(num_stream);
   rxq_conf                  = dev_info.default_rxconf;

   rte_eth_conf port_conf = getPortConf(num_stream, dev_info);
   rxq_conf.offloads      = port_conf.rxmode.offloads;
}

rte_eth_dev_info DpdkSource::getDevInfo(int num_stream)
{
   rte_eth_dev_info dev_info;
   int ret = rte_eth_dev_info_get(streams[num_stream].dpdk_port_id, &dev_info);

   if (ret != 0)
      rte_exit(EXIT_FAILURE, "Error getting dev info of device (port %u) info: %s\n", streams[num_stream].dpdk_port_id, strerror(-ret));

   return dev_info;
}

rte_eth_conf DpdkSource::getPortConf(int num_stream, rte_eth_dev_info &dev_info)
{
   rte_eth_conf port_conf = {};

   port_conf.txmode.offloads &= dev_info.tx_offload_capa;
   configurePort(num_stream, port_conf);
   return port_conf;
}

void DpdkSource::configurePort(int num_stream, rte_eth_conf &port_conf)
{
   int ret = rte_eth_dev_configure(streams[num_stream].dpdk_port_id, definitions::NR_QUEUES, 0, &port_conf);//configure for tx only
   if (ret < 0)
      rte_exit(EXIT_FAILURE, ":: cannot configure device: err=%d, port=%u\n", ret, streams[num_stream].dpdk_port_id);
}

void DpdkSource::setupRxQueues(int num_stream, rte_eth_rxconf &rxq_conf)
{
   for (int i = 0; i < definitions::NR_QUEUES; i++)
   {
      int ret = rte_eth_rx_queue_setup(streams[num_stream].dpdk_port_id,
                                       i,
                                       4096,
                                       rte_eth_dev_socket_id(streams[num_stream].dpdk_port_id),
                                       &rxq_conf,
                                       streams[num_stream].mbuf_pool);
      if (ret < 0)
      {
         rte_exit(EXIT_FAILURE, "Rx queue setup failed: err=%d, port=%u\n", ret, streams[num_stream].dpdk_port_id);
      }
   }
}

void DpdkSource::enablePromiscuousMode(int num_stream)
{
   int ret = rte_eth_promiscuous_enable(streams[num_stream].dpdk_port_id);
   if (ret != 0)
      rte_exit(EXIT_FAILURE, "Promiscuous mode enable failed: err=%s, port=%u\n", rte_strerror(-ret), streams[num_stream].dpdk_port_id);
}

void DpdkSource::startPort(int num_stream)
{
   int ret = rte_eth_dev_start(streams[num_stream].dpdk_port_id);
   if (ret < 0)
      rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n", ret, streams[num_stream].dpdk_port_id);
}

void DpdkSource::assertLinkStatus(int num_stream)
{
   struct rte_eth_link link;
   memset(&link, 0, sizeof(link));

   auto link_get_err = rte_eth_link_get(streams[num_stream].dpdk_port_id, &link);
   if (link_get_err != 0 || link.link_status != RTE_ETH_LINK_UP)
      rte_exit(EXIT_FAILURE, "Link is not up");
}

DpdkSource::~DpdkSource()
{
   stopRXThread();
   rte_eal_cleanup();
}

int DpdkSource::getSamples(int num_stream, int number_of_samples, std::complex<float> *samples)
{
   auto packets_to_dequeue = ((number_of_samples) / definitions::SAMPLES_PER_PACKET) + 1;
   struct rte_mbuf *mbufs[packets_to_dequeue];
   auto total_samples_received = 0;
   auto packets_dequeued = rte_eth_rx_burst(getPortID(num_stream), definitions::SELECTED_QUEUE, mbufs, packets_to_dequeue);

   for (auto packet_idx = 0U; packet_idx < packets_dequeued; packet_idx++)
   {
      struct rte_mbuf *mbuf = mbufs[packet_idx];
      int16_t *payload      = rte_pktmbuf_mtod_offset(mbuf, int16_t *, sizeof(hrzr_packet_all_headers));

      auto samples_received = hrzr_parser->parsePayloadSamples(static_cast<int>(packet_idx), (int16_t *) payload, samples);

      rte_pktmbuf_free((struct rte_mbuf *) mbufs[packet_idx]);
      total_samples_received += samples_received;
   }
   return total_samples_received;
}

int DpdkSource::dumpThread(rx_thread_arg *rx_thread_arg)
{
   DpdkSource *dpdkSource = rx_thread_arg->dpdk_source;
   int num_stream         = rx_thread_arg->num_stream;

   fflush(stdout);
   struct rte_mbuf *mbufs[32];
   struct hrzr_packet_all_headers *packet;
   struct rte_flow_error error;
   uint16_t nb_rx;
   int ret;
   int j;

   int prev_seq = 0;
   int seq      = 0;
   std::list<uint64_t> hrzr_header;
   bool track_payload = true;
   bool track_meta    = true;
   int every_other    = 1;
   std::list<uint32_t> hrzr_payload;
   std::list<uint32_t> hrzr_meta;

   std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

   while (!dpdkSource->shouldQuit())
   {
      nb_rx = rte_eth_rx_burst(dpdkSource->getPortID(num_stream), definitions::SELECTED_QUEUE, mbufs, 32);

      if (nb_rx)
      {
         for (j = 0; j < nb_rx; j++)
         {
            struct rte_mbuf *m = mbufs[j];
            packet             = rte_pktmbuf_mtod(m, struct hrzr_packet_all_headers *);
            prev_seq           = seq;
            seq                = HrzrHeaderParser::getSequenceNumberFromHeader(packet->hrzr);
            dpdkSource->total_packets++;
            if (seq != prev_seq + 1 && prev_seq != 4095 && seq != 0)
            {
               dpdkSource->total_packets_lost += std::abs(seq - prev_seq);
               dpdkSource->total_packets += std::abs(seq - prev_seq);
               int diff = std::abs(seq - prev_seq);
               if (seq < prev_seq)
               {
                  diff = seq + (4095 - prev_seq);
               }
               float delta = (float) dpdkSource->total_packets_lost / dpdkSource->total_packets;

               std::cout << "skipping " << diff << " packets.\t"
                         << " rate: " << delta << std::endl;
            }

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            if (dpdkSource->total_packets % every_other == 0)
               hrzr_header.push_back(packet->hrzr);

            if (track_meta && HrzrHeaderParser::getControlFromHeader(packet->hrzr) == HrzrHeaderParser::PacketType::METADATA)
            {
               uint32_t *meta = rte_pktmbuf_mtod_offset((struct rte_mbuf *) m, uint32_t *, sizeof(hrzr_packet_all_headers));
               for (int i = 0; i < 10; i++) hrzr_meta.push_back(meta[i]);
            }
            else if (track_payload && dpdkSource->total_packets % every_other == 0)
            {
               uint32_t *payload = rte_pktmbuf_mtod_offset((struct rte_mbuf *) m, uint32_t *, sizeof(hrzr_packet_all_headers));
               for (int i = 0; i < definitions::SAMPLES_PER_PACKET; i++) hrzr_payload.push_back(payload[i]);
            }

            if (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() >= 3)
            {
               std::ofstream fs("hrzr_samples.txt");
               while (hrzr_header.size() > 0)
               {
                  std::cout << "REMAINING DATA TO WRITE: " << hrzr_header.size() << std::endl;
                  fs << std::hex << std::setfill('0') << std::setw(16) << hrzr_header.front();
                  if (track_payload && dpdkSource->isValidPacketType(HrzrHeaderParser::getControlFromHeader(hrzr_header.front())))
                  {
                     for (int i = 0; i < definitions::SAMPLES_PER_PACKET; i++)
                     {
                        fs << std::hex << std::setfill('0') << std::setw(8) << hrzr_payload.front();
                        hrzr_payload.pop_front();
                     }
                  }
                  if (track_meta && HrzrHeaderParser::getControlFromHeader(hrzr_header.front()) == HrzrHeaderParser::PacketType::METADATA)
                  {
                     for (int i = 0; i < 10; i++)
                     {
                        fs << std::hex << std::setfill('0') << std::setw(8) << hrzr_meta.front();
                        hrzr_meta.pop_front();
                     }
                  }
                  fs << std::endl;
                  hrzr_header.pop_front();
               }
               std::cout << "DONE" << std::endl;
               dpdkSource->quit = true;
            }

            rte_pktmbuf_free(m);
         }
      }
   }

   rte_flow_flush(dpdkSource->getPortID(num_stream), &error);
   ret = rte_eth_dev_stop(dpdkSource->getPortID(num_stream));
   if (ret < 0)
      printf("Failed to stop port %u: %s", dpdkSource->getPortID(num_stream), rte_strerror(-ret));
   rte_eth_dev_close(dpdkSource->getPortID(num_stream));

   return 0;
}

int DpdkSource::RXThread(rx_thread_arg *rx_thread_arg)
{
   DpdkSource *dpdkSource = rx_thread_arg->dpdk_source;
   int num_stream         = rx_thread_arg->num_stream;

   struct rte_mbuf *mbufs[32];
   struct hrzr_packet_all_headers *packet;
   struct rte_flow_error error;
   int ret;
   int j;

   int prev_seq;
   int seq = 0;

   const auto port_id = dpdkSource->getPortID(num_stream);

   while (!dpdkSource->shouldQuit())
   {
      auto nb_rx = rte_eth_rx_burst(port_id, definitions::SELECTED_QUEUE, mbufs, 32);
      for (j = 0; j < nb_rx; j++)
      {
         struct rte_mbuf *m = mbufs[j];
         packet             = rte_pktmbuf_mtod(m, struct hrzr_packet_all_headers *);
         prev_seq           = seq;
         seq                = HrzrHeaderParser::getSequenceNumberFromHeader(packet->hrzr);
         dpdkSource->total_packets++;
         if (seq != prev_seq + 1 && prev_seq != 4095 && seq != 0)
         {
            dpdkSource->total_packets_lost += std::abs(seq - prev_seq);
            dpdkSource->total_packets += std::abs(seq - prev_seq);
            int diff = std::abs(seq - prev_seq);
            if (seq < prev_seq)
            {
               diff = seq + (4095 - prev_seq);
            }
            float delta = (float) dpdkSource->total_packets_lost / dpdkSource->total_packets;

            std::cout << "stream " << num_stream << " skipping " << diff << " packets.\t"
                      << " rate: " << delta << std::endl;
         }

         //rte_pktmbuf_dump(stdout, m, sizeof(struct hrzr_packet_all_headers));
         if (packet->ipv4_hdr.next_proto_id == IPPROTO_UDP && packet->ether_hdr.ether_type == htons(RTE_ETHER_TYPE_IPV4)
             && packet->udp.dst_port == htons(dpdkSource->getUdpRxPort(num_stream))
             && m->pkt_len == definitions::PAYLOAD_SIZE + sizeof(struct hrzr_packet_all_headers)
             && dpdkSource->isValidPacketType(HrzrHeaderParser::getControlFromHeader(packet->hrzr)))
         {
            rte_ring_enqueue(dpdkSource->getRteRing(num_stream), (void *) m);
         }
         else
         {
            rte_pktmbuf_free(m);
         }
      }
   }

   rte_flow_flush(dpdkSource->getPortID(num_stream), &error);
   ret = rte_eth_dev_stop(dpdkSource->getPortID(num_stream));
   if (ret < 0)
      printf("Failed to stop port %u: %s", dpdkSource->getPortID(num_stream), rte_strerror(-ret));
   rte_eth_dev_close(dpdkSource->getPortID(num_stream));

   return 0;
}

void DpdkSource::stopRXThread()
{
   quit = true;
   rte_eal_wait_lcore(1);
}

bool DpdkSource::isValidPacketType(HrzrHeaderParser::PacketType type)
{
   if (type == HrzrHeaderParser::PacketType::DATA || type == HrzrHeaderParser::PacketType::DATA_END_OF_BURST)
      return true;
   return false;
}