#ifndef __LC_TRIE_BGP_H__
#define __LC_TRIE_BGP_H__
// begin #ifndef guard

#include <stdlib.h>
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

// read the subnet to ASN file
// return number of entries read
// return negative on failure
extern int
read_prefix_table(char *filename,
                  lct_bgp_prefix_t prefix[],
                  size_t prefix_size);

// three-way subnet comparison for qsort
extern int prefix_cmp(lct_bgp_prefix_t **i, lct_bgp_prefix_t **j);

// read the ASN to description file
// return number of entries read
// return negative on failure
extern int
read_asn_table(char *filename,
               lct_bgp_asn_t prefix[],
               size_t prefix_size);

// end #ifndef guard
#endif

