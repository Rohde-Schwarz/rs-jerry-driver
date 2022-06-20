#include "dpdkSource.h"

#include "common/definitions.h"

#include <iostream>
#include <vector>

DpdkSource::DpdkSource(int dpdk_port_id, uint16_t udp_rx_port, float norm)
   : hrzr_parser(std::make_unique<HrzrParser>(norm))
   , dpdk_port_id(dpdk_port_id)
   , quit(false)
   , udp_rx_port(udp_rx_port)
{
   setupDpdk();
   setupPort();
   setupRing();
   startRXThread();
}

DpdkSource::~DpdkSource()
{
   stopRXThread();
   rte_eal_cleanup();
}

int DpdkSource::getSamples(int number_of_samples, std::complex<float> *samples)
{
   auto packets_to_dequeue = ((number_of_samples) / definitions::SAMPLES_PER_PACKET) + 1;
   struct rte_mbuf *mbufs[packets_to_dequeue];
   auto total_samples_received = 0;
   auto packets_dequeued       = rte_ring_sc_dequeue_burst(ring, (void **) mbufs, packets_to_dequeue, NULL);

   for (int packet_idx = 0; packet_idx < packets_dequeued; packet_idx++)
   {
      int16_t *payload = rte_pktmbuf_mtod_offset((struct rte_mbuf *) mbufs[packet_idx], int16_t *, sizeof(hrzr_packet_all_headers));

      auto samples_received = hrzr_parser->parsePayloadSamples(packet_idx, (int16_t *) payload, samples);

      rte_pktmbuf_free((struct rte_mbuf *) mbufs[packet_idx]);
      total_samples_received += samples_received;
   }
   return total_samples_received;
}

void DpdkSource::setupDpdk()
{
   int ret;
   uint16_t nr_ports;
   struct rte_flow_error error;
   std::vector<const char *> argv{"dpdk", "-l", "0,1"};
   //char *argv[] = {"dpdk", "-l", "0,1", 0};
   int argc = 3;
   /* Initialize EAL. 8< */
   printf("EAL init start.\n");
   fflush(stdout);
   ret = rte_eal_init(argc, (char **) argv.data());
   if (ret < 0)
      rte_exit(EXIT_FAILURE, ":: invalid EAL arguments\n");
   /* >8 End of Initialization of EAL. */

   nr_ports = rte_eth_dev_count_avail();
   if (nr_ports == 0)
      rte_exit(EXIT_FAILURE, ":: no Ethernet ports found\n");

   if (nr_ports != 1)
   {
      printf(":: warn: %d ports detected, but we use only one: port %u\n", nr_ports, dpdk_port_id);
   }

   /* Allocates a mempool to hold the mbufs. 8< */
   mbuf_pool = rte_pktmbuf_pool_create("mbuf_pool", 1024, 0, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
   /* >8 End of allocating a mempool to hold the mbufs. */

   printf("setupDpdk done.\n");
   if (mbuf_pool == NULL)
      rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");
}

void DpdkSource::setupPort()
{
   int ret;
   uint16_t i;
   /* Ethernet port configured with default settings. 8< */
   struct rte_eth_conf port_conf = {
       .rxmode =
           {
               .split_hdr_size = 0,
           },
   };
   struct rte_eth_rxconf rxq_conf;
   struct rte_eth_dev_info dev_info;

   ret = rte_eth_dev_info_get(dpdk_port_id, &dev_info);
   if (ret != 0)
      rte_exit(EXIT_FAILURE, "Error during getting device (port %u) info: %s\n", dpdk_port_id, strerror(-ret));

   port_conf.txmode.offloads &= dev_info.tx_offload_capa;
   printf(":: initializing port: %d\n", dpdk_port_id);

   ret = rte_eth_dev_configure(dpdk_port_id, definitions::NR_QUEUES, 0, &port_conf);//configure for tx only
   if (ret < 0)
   {
      rte_exit(EXIT_FAILURE, ":: cannot configure device: err=%d, port=%u\n", ret, dpdk_port_id);
   }

   rxq_conf          = dev_info.default_rxconf;
   rxq_conf.offloads = port_conf.rxmode.offloads;
   /* >8 End of ethernet port configured with default settings. */

   /* Configuring RX queue connected to single port. 8< */
   for (i = 0; i < definitions::NR_QUEUES; i++)
   {
      ret = rte_eth_rx_queue_setup(dpdk_port_id, i, 512, rte_eth_dev_socket_id(dpdk_port_id), &rxq_conf, mbuf_pool);
      if (ret < 0)
      {
         rte_exit(EXIT_FAILURE, ":: Rx queue setup failed: err=%d, port=%u\n", ret, dpdk_port_id);
      }
   }

   /* Setting the RX port to promiscuous mode. 8< */
   ret = rte_eth_promiscuous_enable(dpdk_port_id);
   if (ret != 0)
      rte_exit(EXIT_FAILURE, ":: promiscuous mode enable failed: err=%s, port=%u\n", rte_strerror(-ret), dpdk_port_id);
   /* >8 End of setting the RX port to promiscuous mode. */

   /* Starting the port. 8< */
   ret = rte_eth_dev_start(dpdk_port_id);
   if (ret < 0)
   {
      rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n", ret, dpdk_port_id);
   }
   /* >8 End of starting the port. */
   assertLinkStatus();

   printf(":: initializing port: %d done\n", dpdk_port_id);
}

void DpdkSource::setupRing()
{
   printf("setting up rx ringbuffer\n");
   ring = rte_ring_create("rx_ring", definitions::RING_SIZE, rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);
}

void DpdkSource::startRXThread()
{
   rte_eal_remote_launch((lcore_function_t *) RXThread, this, 1);
   printf("rx thread started\n");
}

int DpdkSource::RXThread(DpdkSource *dpdkSource)
{

   printf("rx thread started, udp rx port %d\n", dpdkSource->getUdpRxPort());
   fflush(stdout);
   struct rte_mbuf *mbufs[32];
   struct hrzr_packet_all_headers *packet;
   struct rte_flow_error error;
   uint16_t nb_rx;
   int ret;
   int j;

   while (!dpdkSource->shouldQuit())
   {
      nb_rx = rte_eth_rx_burst(dpdkSource->getPortID(), definitions::SELECTED_QUEUE, mbufs, 32);
      if (nb_rx)
      {
         for (j = 0; j < nb_rx; j++)
         {
            struct rte_mbuf *m = mbufs[j];
            packet             = rte_pktmbuf_mtod(m, struct hrzr_packet_all_headers *);

            short sequence_number = HrzrHeaderParser::getSequenceNumberFromHeader(packet->hrzr);

            //rte_pktmbuf_dump(stdout, m, sizeof(struct hrzr_packet_all_headers));
            if (packet->ipv4_hdr.next_proto_id == IPPROTO_UDP && packet->ether_hdr.ether_type == htons(RTE_ETHER_TYPE_IPV4)
                && packet->udp.dst_port == htons(dpdkSource->getUdpRxPort()) && m->pkt_len == definitions::PAYLOAD_SIZE + sizeof(struct hrzr_packet_all_headers)
                && dpdkSource->isValidPacketType(HrzrHeaderParser::getControlFromHeader(packet->hrzr)))
            {
               rte_ring_enqueue(dpdkSource->getRteRing(), (void *) m);
            }
            else
            {
               rte_pktmbuf_free(m);
            }
         }
      }
   }

   /* >8 End of reading the packets from all queues. */
   /* closing and releasing resources */

   rte_flow_flush(dpdkSource->getPortID(), &error);
   ret = rte_eth_dev_stop(dpdkSource->getPortID());
   if (ret < 0)
      printf("Failed to stop port %u: %s", dpdkSource->getPortID(), rte_strerror(-ret));
   rte_eth_dev_close(dpdkSource->getPortID());

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

void DpdkSource::assertLinkStatus(void)
{
   struct rte_eth_link link = {0};
   int link_get_err         = -EINVAL;

   link_get_err = rte_eth_link_get(dpdk_port_id, &link);
   if (link_get_err != 0 || link.link_status != ETH_LINK_UP)
      rte_exit(EXIT_FAILURE, "Link is not up");
}
