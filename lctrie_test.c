#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>

#include "lctrie_ip.h"
#include "lctrie_bgp.h"
#include "lctrie.h"

#define BGP_MAX_ENTRIES 4000000

#define LCT_IP_DISPLAY_PREFIXES     0

void print_subnet(lct_subnet_t *subnet) {
  char pstr[INET_ADDRSTRLEN];
  uint32_t prefix;

  if (!subnet)
    return;

  prefix = htonl(subnet->addr);
  if (!inet_ntop(AF_INET, &(prefix), pstr, sizeof(pstr))) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    return;
  }

  switch (subnet->info.type) {
    case IP_SUBNET_BGP:
      printf("BGP%s prefix %s/%d for ASN %d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len,  subnet->info.bgp.asn);
      break;

    case IP_SUBNET_PRIVATE:
      printf("Private class %c%s subnet for %s/%d\n", subnet->info.priv.class, subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_LINKLOCAL:
      printf("Link local%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_MULTICAST:
      printf("Multicast%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_BROADCAST:
      printf("Broadcast%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_LOOPBACK:
      printf("Loopback%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_RESERVED:
      printf("Reserved%s subnet for %s/%d, %s\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len, subnet->info.rsv.desc);
      break;

    case IP_SUBNET_BOGON:
      printf("Bogon%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;


    default:
      printf("Invalid prefix type for %s/%d\n", pstr, subnet->len);
      break;
  }
}

void print_subnet_stats(lct_subnet_t *subnet, lct_ip_stats_t *stats) {
  char pstr[INET_ADDRSTRLEN];
  uint32_t prefix;

  if (!subnet || !stats)
    return;

  prefix = htonl(subnet->addr);
  if (!inet_ntop(AF_INET, &(prefix), pstr, sizeof(pstr))) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    return;
  }

  switch (subnet->info.type) {
    case IP_SUBNET_BGP:
      printf("BGP%s prefix %s/%d (%d/%d) for ASN %d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len, stats->used, stats->size,  subnet->info.bgp.asn);
      break;

    case IP_SUBNET_PRIVATE:
      printf("Private class %c%s subnet for %s/%d\n", subnet->info.priv.class, subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_LINKLOCAL:
      printf("Link local%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_MULTICAST:
      printf("Multicast%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_BROADCAST:
      printf("Broadcast%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_LOOPBACK:
      printf("Loopback%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;

    case IP_SUBNET_RESERVED:
      printf("Reserved%s subnet for %s/%d, %s\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len, subnet->info.rsv.desc);
      break;

    case IP_SUBNET_BOGON:
      printf("Bogon%s subnet for %s/%d\n", subnet->type == IP_PREFIX_FULL ? " FULL" : "", pstr, subnet->len);
      break;


    default:
      printf("Invalid prefix type for %s/%d\n", pstr, subnet->len);
      break;
  }
}

int main(int argc, char *argv[]) {
  int num = 0;
  int nprefixes = 0, nbases = 0, nfull = 0;
  lct_subnet_t *p;
  lct_t t;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <BGP ASN Subnets> <BGP ASN Assignments>\n", basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  if (!(p = (lct_subnet_t *)calloc(sizeof(lct_subnet_t), BGP_MAX_ENTRIES))) {
    fprintf(stderr, "Could not allocate subnet input buffer\n");
    exit(EXIT_FAILURE);
  }

  // fill up the rest of the array with reserved IP subnets
  num += init_reserved_subnets(p, BGP_MAX_ENTRIES);

  // read in the ASN prefixes
  int rc;
  printf("Reading prefixes from %s...\n\n", argv[1]);
  if (0 > (rc = read_prefix_table(argv[1], &p[num], BGP_MAX_ENTRIES - num))) {
    fprintf(stderr, "could not read prefix file \"%s\"\n", argv[1]);
    return rc;
  }
  num += rc;

  // validate subnet prefixes against their netmasks
  // and sort the resulting array
  subnet_mask(p, num);
  qsort(p, num, sizeof(lct_subnet_t), subnet_cmp);

  // de-duplicate subnets and shrink the buffer down to its
  // actual size and split into prefixes and bases
  num -= subnet_dedup(p, num);
  p = realloc(p, num * sizeof(lct_subnet_t));

  // allocate a buffer for the IP stats
  lct_ip_stats_t *stats = (lct_ip_stats_t *) calloc(num, sizeof(lct_ip_stats_t));
  if (!stats) {
    fprintf(stderr, "Failed to allocate prefix statistics buffer\n");
    return 0;
  }

  // count which subnets are prefixes of other subnets
  nprefixes = subnet_prefix(p, stats, num);
  nbases = num - nprefixes;

  // we're storing twice as many subnets as necessary for easy
  // iteration over the entire sorted subnet list.
  printf("Enumerating database, get ready! 3..2..1..GO!!!\n\n");
  for (int i = 0; i < num; i++) {
#if LCT_IP_DISPLAY_PREFIXES
    print_subnet_stats(&p[i], &stats[i]);
#endif

    // count up the full prefixes to calculate the savings on trie nodes
    if (p[i].type == IP_PREFIX_FULL)
      ++nfull;

    // quick error check on the optimized prefix indexes
    uint32_t prefix = p[i].prefix;
    if (prefix != IP_PREFIX_NIL && p[prefix].type == IP_PREFIX_FULL) {
      printf("ERROR: optimized subnet index points to a full prefix\n");
    }
  }

  uint32_t subnet_bytes = num * sizeof(lct_subnet_t);
  uint32_t stats_bytes = num * sizeof(lct_ip_stats_t);
  printf("\nStats:\n");
  printf("Read %d unique subnets using %u %s memory for subnet descriptors and %u %s for ephemeral IP stats.\n",
         num,
         subnet_bytes / ((subnet_bytes > 1024) ? (subnet_bytes > 1024 * 1024) ? 1024 * 1024 : 1024 : 1),
         (subnet_bytes > 1024) ? (subnet_bytes > 1024 * 1024) ? "mB" : "kB" : "B",
         stats_bytes / ((stats_bytes > 1024) ? (stats_bytes > 1024 * 1024) ? 1024 * 1024 : 1024 : 1),
         (stats_bytes > 1024) ? (stats_bytes > 1024 * 1024) ? "mB" : "kB" : "B");
  printf("%d subnets are fully allocated to subprefixes culling %1.2f%% subnets from the match count.\n",
         nfull, (100.0f * nfull) / num);
  printf("%d optimized prefixes of %d base subnets will make a trie with %1.2f%% base leaf nodes.\n",
         nprefixes - nfull, nbases, (100.0f * nbases) / (num - nfull));
  printf("The trie will consist of %1.2f%% base subnets and %1.2f%% total subnets from the full subnet list.\n",
         (100.0f * nbases) / (num), (100.0f * (num - nfull)) / num);

  // actually build the trie and get the trie node count for statistics printing
  memset(&t, 0, sizeof(lct_t));
  lct_build(&t, p, num);
  uint32_t node_bytes = t.ncount * sizeof(lct_node_t) + t.bcount * sizeof(uint32_t);
  printf("The resulting trie has %u nodes using %u %s memory.\n", t.ncount,
         node_bytes / ((node_bytes > 1024) ? (node_bytes > 1024 * 1024) ? 1024 * 1024 : 1024 : 1),
         (node_bytes > 1024) ? (node_bytes > 1024 * 1024) ? "mB" : "kB" : "B");
 
  // TODO run some basic tests with known data sets to test that we're matching base subnets, prefix subnets
  //
  // TODO run some performance tests by looping for an interval and counting how many lookups we can make in
  //      that period.  Tally up the address types matched and print those statistics.

  printf("\nHit enter key to continue...\n");
  getc(stdin);

  // we're done with the subnets, stats, and trie;  dump them.
  lct_free(&t);
  free(stats);
  free(p);

  return 0;
}
