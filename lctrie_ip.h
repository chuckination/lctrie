#ifndef __LC_TRIE_IP_H__
#define __LC_TRIE_IP_H__
// begin #ifndef guard

#include <stdlib.h>
#include <stdint.h>

#define IP_SUBNET_UNUSED    0
#define IP_SUBNET_BGP        1
#define IP_SUBNET_PRIVATE    2
#define IP_SUBNET_LINKLOCAL  3
#define IP_SUBNET_MULTICAST  4
#define IP_SUBNET_BROADCAST  5
#define IP_SUBNET_LOOPBACK  6
#define IP_SUBNET_RESERVED  7
#define IP_SUBNET_BOGON     8
#define IP_SUBNET_USER      9

// Bit manipulation macros
//
// Extract num bits from 32 bit string starting at pos bit
// requires bit strings to be in host endian byte order
// for the bit manipulation to work properly
#define EXTRACT(pos, num, str) ((str)<<(pos)>>(32-(num)))

// link local, multicast, loopback, and reserved have no additional
// information and thereforce only have a type

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

// RFC5735 reserved IP subnets
typedef struct lct_subnet_reserved_t {
  uint32_t type;
  const char *desc;
} lct_subnet_reserved_t;

// User customized IP subnets (host groupings)
typedef struct lct_subnet_usr_t {
  uint32_t type;
  void *data;
} lct_subnet_usr_t;

// union representing all possible subnet info types
typedef union lct_subnet_info {
  uint32_t type;
  lct_subnet_bgp_t bgp;
  lct_subnet_private_t priv;
  lct_subnet_reserved_t rsv;
  lct_subnet_usr_t usr;
} lct_subnet_info_t;

// the actual IP subnet structure
typedef struct lct_subnet {
  uint32_t addr;
  uint32_t len;

  // pointer to our next highest prefix
  struct lct_subnet *prefix;

  lct_subnet_info_t info;
} lct_subnet_t;

// fill in user array with reserved IP subnets
// according to RFC 5735
extern int
init_reserved_subnets(lct_subnet_t prefix[],
                      size_t prefix_size);

// three-way subnet comparison for qsort
extern int subnet_cmp(const void *di, const void *dj);

// is subnet s a prefix of the subnet t?
// requires the two elements to be sorted and in order according
// to subnet_cmp
extern int subnet_isprefix(lct_subnet_t *s, lct_subnet_t *t);

// end #ifndef guard
#endif
