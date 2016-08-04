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

#define LCT_IP_READ_PREFIXES_FILES  1
#define LCT_IP_DISPLAY_PREFIXES     1

int main(int argc, char *argv[]) {
  int num = 0;
  lct_subnet_t *p;
  int nprefixes = 0, nbases = 0, nfull = 0;

#if LCT_IP_DISPLAY_PREFIXES
  char pstr[INET_ADDRSTRLEN];
  uint32_t prefix;
#endif

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

#if LCT_IP_READ_PREFIXES_FILES
  // read in the ASN prefixes
  int rc;
  printf("Reading prefixes from %s...\n\n", argv[1]);
  if (0 > (rc = read_prefix_table(argv[1], &p[num], BGP_MAX_ENTRIES - num))) {
    fprintf(stderr, "could not read prefix file \"%s\"\n", argv[1]);
    return rc;
  }
  num += rc;
#endif

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

#if LCT_IP_DISPLAY_PREFIXES
  // we're storing twice as many subnets as necessary for easy
  // iteration over the entire sorted subnet list.
  printf("Enumerating database, get ready! 3..2..1..GO!!!\n\n");
  for (int i = 0; i < num; i++) {
    prefix = htonl(p[i].addr);
    if (!inet_ntop(AF_INET, &(prefix), pstr, sizeof(pstr))) {
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      continue;
    }

    switch (p[i].info.type) {
      case IP_SUBNET_BGP:
        printf("BGP%s prefix %s/%d (%d/%d) for ASN %d\n", p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len, stats[i].used, stats[i].size,  p[i].info.bgp.asn);
        break;

      case IP_SUBNET_PRIVATE:
        printf("Private class %c%s subnet for %s/%d\n", p[i].info.priv.class, p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len);
        break;

      case IP_SUBNET_LINKLOCAL:
        printf("Link local%s subnet for %s/%d\n", p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len);
        break;

      case IP_SUBNET_MULTICAST:
        printf("Multicast%s subnet for %s/%d\n", p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len);
        break;

      case IP_SUBNET_BROADCAST:
        printf("Broadcast%s subnet for %s/%d\n", p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len);
        break;

      case IP_SUBNET_LOOPBACK:
        printf("Loopback%s subnet for %s/%d\n", p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len);
        break;

      case IP_SUBNET_RESERVED:
        printf("Reserved%s subnet for %s/%d, %s\n", p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len, p[i].info.rsv.desc);
        break;

      case IP_SUBNET_BOGON:
        printf("Bogon%s subnet for %s/%d\n", p[i].type == IP_PREFIX_FULL ? " FULL" : "", pstr, p[i].len);
        break;


      default:
        printf("Invalid prefix type for %s/%d\n", pstr, p[i].len);
        break;
    }

    // count up the full prefixes to calculate the savings on trie nodes
    if (p[i].type == IP_PREFIX_FULL)
      ++nfull;

    // quick error check on the optimized prefix indexes
    uint32_t prefix = p[i].prefix;
    if (prefix != IP_PREFIX_NIL && p[prefix].type == IP_PREFIX_FULL) {
      printf("ERROR: optimized subnet index points to a full prefix\n");
    }
  }
#endif

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
  printf("%d optimized prefixes of %d base subnets in the possible to match.\n",
         nprefixes - nfull, nbases);

  // we're done with the statistics and subnets, dump them.
  free(stats);
  free(p);

  return 0;
}
