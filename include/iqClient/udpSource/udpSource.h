#include "../common/definitions.h"
#include "../dpdkSource/hrzrParser.h"
#include "../linuxSource.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER "192.168.50.1"
#define PORT 12//The port on which to send data

class UdpSource : public ILinuxSource {
public:
   UdpSource(float norm);
   ~UdpSource() override;
   int getSamples(int number_of_samples, std::complex<float> *samples) override;
   void setIp(std::string ip);
   void setPort(int port);
   void connect();
   void disconnect();

private:
   std::string ip;
   int port;
   char buf[definitions::PACKET_SIZE];
   struct sockaddr_in si_other;
   int s, slen = sizeof(si_other);
   std::unique_ptr<HrzrParser> hrzr_parser;

   void die(const char *s);
};