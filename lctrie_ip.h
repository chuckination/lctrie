#ifndef __LC_TRIE_IP_H__
#define __LC_TRIE_IP_H__
// begin #ifndef guard

#include <stdlib.h>
#include <stdint.h>

#define IP_SUBNET_UNUSED		0
#define IP_SUBNET_BGP				1
#define IP_SUBNET_PRIVATE		2
#define IP_SUBNET_LINKLOCAL	3

// subnet bgp has a 32-bit AS number
typedef struct lct_subnet_bgp_t {
	uint32_t type;
	uint32_t asn;
} lct_subnet_bgp_t;

// RFC1918 private IP subnets have a 
typedef struct lct_subnet_private_t {
	uint32_t type;
	char class;
} lct_subnet_private_t;

// union representing all possible subnet info types
typedef union lct_subnet_info {
	uint32_t type;
	lct_subnet_bgp_t bgp;
	lct_subnet_private_t priv;
} lct_subnet_info_t; 

// the actual IP subnet structure
typedef struct lct_subnet {
	uint32_t prefix;
	uint32_t len;

	lct_subnet_info_t info;
} lct_subnet_t;

// fill in user array with reserved IP subnets
// RFC1918 for private IP reservations
// RFC3927 for link local IP reservations
extern int
init_reserved_subnets(lct_subnet_t prefix[],
											size_t prefix_size);

// TODO fix this for glibc's qsort
// three-way subnet comparison for qsort
extern int subnet_cmp(const void *di, const void *dj);

// end #ifndef guard
#endif
