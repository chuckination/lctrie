#include "lctrie_ip.h"

#include <stdio.h>

#include <arpa/inet.h>

int subnet_cmp(const void *di, const void *dj) {
	const lct_subnet_t *i = (const lct_subnet_t *) di;
	const lct_subnet_t *j = (const lct_subnet_t *) dj;

	if (i->prefix < j->prefix)
		return -1;
	else if (i->prefix > j->prefix)
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
	// There's 4 reserved subnets
	//
	// 3 subnets for RFC1918 private IP reservations
	// Class A - 10.0.0.0/8
	// Class B - 172.16.0.0/12
	// Class C - 192.168.0.0/16
	//
	// 1 subnet for link local (APIPA) IP reservations
	// LL/APIPA	-	169.254.0.0/16
	
	if (prefix_size < 4) {
		fprintf(stderr, "Need a prefix buffer of size 4 for reserved ranges\n");
		return -1;
	}

	// just build the reservations by hand

	prefix[0].info.type = IP_SUBNET_PRIVATE;
	prefix[0].info.priv.class = 'a';
	inet_pton(AF_INET, "10.0.0.0", &(prefix[0].prefix));
	prefix[0].len = 8;

	prefix[1].info.type = IP_SUBNET_PRIVATE;
	prefix[1].info.priv.class = 'b';
	inet_pton(AF_INET, "172.16.0.0", &(prefix[1].prefix));
	prefix[1].len = 12;

	prefix[2].info.type = IP_SUBNET_PRIVATE;
	prefix[2].info.priv.class = 'c';
	inet_pton(AF_INET, "192.168.0.0", &(prefix[2].prefix));
	prefix[2].len = 16;

	prefix[3].info.type = IP_SUBNET_LINKLOCAL;
	inet_pton(AF_INET, "169.254.0.0", &(prefix[3].prefix));
	prefix[3].len = 16;

	return 4;
}
