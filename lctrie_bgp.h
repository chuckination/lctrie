#ifndef __LC_TRIE_BGP_H__
#define __LC_TRIE_BGP_H__
// begin #ifndef guard

#include <stdint.h>

typedef struct lct_bgp_prefix {
	// Format: 72.252.216.0/21	30689
	uint32_t prefix;
	uint32_t len;
	uint32_t asn;
	uint32_t reserved;
} lct_bgp_prefix_t;

typedef struct lct_bgp_asn {
	uint32_t num;
	char *desc;
} lct_bgp_asn_t;

lct_bgp_prefix_t* read_prefix_table();
lct_bgp_asn_t* read_asn_table();

// end #ifndef guard
#endif

