#include "lctrie_ip.h"

#include <stdio.h>

#include <arpa/inet.h>

int subnet_cmp(const void *di, const void *dj) {
	const lct_subnet_t *i = (const lct_subnet_t *) di;
	const lct_subnet_t *j = (const lct_subnet_t *) dj;

	if (ntohl(i->prefix) < ntohl(j->prefix))
		return -1;
	else if (ntohl(i->prefix) > ntohl(j->prefix))
		return 1;
	else if (i->len < j->len)
		return -1;
	else if (i->len > j->len)
		return 1;
	else
		return 0;
}

int
init_reserved_subnets(lct_subnet_t prefix[],
											size_t prefix_size) {
	if (prefix_size < 15) {
		fprintf(stderr, "Need a prefix buffer of size 15 for reserved ranges\n");
		return -1;
	}

  // RFC 5735
	// just build the reservations by hand in order

  // Various reserved addresses according to RFC 5735
  //

  // RFC 1122, Sect. 3.2.1.3 "This" Networks
  //
	prefix[0].info.type = IP_SUBNET_RESERVED;
	prefix[0].info.rsv.desc = "RFC 1122, Sect. 3.2.1.3 \"This\" Networks";
	inet_pton(AF_INET, "0.0.0.0", &(prefix[0].prefix));
	prefix[0].len = 8;

  // RFC 1918 Class A Private Addresses
  //
	prefix[1].info.type = IP_SUBNET_PRIVATE;
	prefix[1].info.priv.class = 'a';
	inet_pton(AF_INET, "10.0.0.0", &(prefix[1].prefix));
	prefix[1].len = 8;

  // RFC 1122, Sect. 3.2.1.3 Loopback
  //
	prefix[2].info.type = IP_SUBNET_LOOPBACK;
	inet_pton(AF_INET, "127.0.0.0", &(prefix[2].prefix));
	prefix[2].len = 8;

  // RFC 3927 Link Local Addresses
  //
	prefix[3].info.type = IP_SUBNET_LINKLOCAL;
	inet_pton(AF_INET, "169.254.0.0", &(prefix[3].prefix));
	prefix[3].len = 16;

  // RFC 1918 Class B Private Addresses
  //
	prefix[4].info.type = IP_SUBNET_PRIVATE;
	prefix[4].info.priv.class = 'b';
	inet_pton(AF_INET, "172.16.0.0", &(prefix[4].prefix));
	prefix[4].len = 12;

  // RFC 5736 IETF Protocol Assignments
  //
	prefix[5].info.type = IP_SUBNET_RESERVED;
	prefix[5].info.rsv.desc = "RFC 5736 IETF Protocol Assignments";
	inet_pton(AF_INET, "192.0.0.0", &(prefix[5].prefix));
	prefix[5].len = 24;

  // RFC 5737 TEST-NET-1
  //
	prefix[6].info.type = IP_SUBNET_RESERVED;
	prefix[6].info.rsv.desc = "RFC 5737 TEST-NET-1";
	inet_pton(AF_INET, "192.0.2.0", &(prefix[6].prefix));
	prefix[6].len = 24;

  // RFC 3068 6to4 Relay Anycast
  //
	prefix[7].info.type = IP_SUBNET_RESERVED;
	prefix[7].info.rsv.desc = "RFC 3068 6to4 Relay Anycast";
	inet_pton(AF_INET, "192.88.99.0", &(prefix[7].prefix));
	prefix[7].len = 16;

  // RFC 1918 Class C Private Addresses
  // 
	prefix[8].info.type = IP_SUBNET_PRIVATE;
	prefix[8].info.priv.class = 'c';
	inet_pton(AF_INET, "192.168.0.0", &(prefix[8].prefix));
	prefix[8].len = 16;

  // RFC 2544 Network Interconnect Device Benchmark Testing
  //
	prefix[9].info.type = IP_SUBNET_RESERVED;
	prefix[9].info.rsv.desc = "RFC 2544 Network Interconnect Device Benchmark Testing";
	inet_pton(AF_INET, "198.18.0.0", &(prefix[9].prefix));
	prefix[9].len = 15;

  // RFC 5737 TEST-NET-2
  //
	prefix[10].info.type = IP_SUBNET_RESERVED;
	prefix[10].info.rsv.desc = "RFC 5737 TEST-NET-2";
	inet_pton(AF_INET, "198.51.100.0", &(prefix[10].prefix));
	prefix[10].len = 24;

  // RFC 5737 TEST-NET-3
  //
	prefix[11].info.type = IP_SUBNET_RESERVED;
	prefix[11].info.rsv.desc = "RFC 5737 TEST-NET-3";
	inet_pton(AF_INET, "203.0.113.0", &(prefix[11].prefix));
	prefix[11].len = 24;

  // RFC 3171 Multicast Addresses
  //
	prefix[12].info.type = IP_SUBNET_MULTICAST;
	inet_pton(AF_INET, "224.0.0.0", &(prefix[12].prefix));
	prefix[12].len = 4;

  // RFC 1112, Section 4 Reserved for Future Use
  //
	prefix[13].info.type = IP_SUBNET_RESERVED;
	prefix[13].info.rsv.desc = "RFC 1112, Section 4 Reserved for Future Use";
	inet_pton(AF_INET, "240.0.0.0", &(prefix[13].prefix));
	prefix[13].len = 4;

  // RFC 919/922, Section 7 Limited Broadcast Address
  //
	prefix[14].info.type = IP_SUBNET_BROADCAST;
	inet_pton(AF_INET, "255.255.255.255", &(prefix[14].prefix));
	prefix[14].len = 32;

	return 15;
}
