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
  printf("Reading prefixes from %s...\n", argv[1]);
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

  // count which subnets are prefixes of other subnets
  nprefixes = subnet_prefix(p, num);
  nbases = num - nprefixes;

#if LCT_IP_DISPLAY_PREFIXES
  // we're storing twice as many subnets as necessary for easy
  // iteration over the entire sorted subnet list.
  for (int i = 0; i < num; i++) {
    prefix = htonl(p[i].addr);
    if (!inet_ntop(AF_INET, &(prefix), pstr, sizeof(pstr))) {
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      continue;
    }

    switch (p[i].info.type) {
      case IP_SUBNET_BGP:
        printf("BGP prefix %s/%d of size %u/%u%s for ASN %d\n", pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "",  p[i].info.bgp.asn);
        break;

      case IP_SUBNET_PRIVATE:
        printf("Private class %c subnet for %s/%d of size %u/%u%s\n", p[i].info.priv.class, pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "");
        break;

      case IP_SUBNET_LINKLOCAL:
        printf("Link local subnet for %s/%d of size %u/%u%s\n", pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "");
        break;

      case IP_SUBNET_MULTICAST:
        printf("Multicast subnet for %s/%d of size %u/%u%s\n", pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "");
        break;

      case IP_SUBNET_BROADCAST:
        printf("Broadcast subnet for %s/%d of size %u/%u%s\n", pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "");
        break;

      case IP_SUBNET_LOOPBACK:
        printf("Loopback subnet for %s/%d of size %u/%u%s\n", pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "");
        break;

      case IP_SUBNET_RESERVED:
        printf("Reserved subnet for %s/%d of size %u/%u%s, %s\n", pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "", p[i].info.rsv.desc);
        break;

      case IP_SUBNET_BOGON:
        printf("Bogon subnet for %s/%d of size %u/%u%s\n", pstr, p[i].len, p[i].used, p[i].size, p[i].used == p[i].size ? " FULL" : "");
        break;


      default:
        printf("Invalid prefix type for %s/%d\n", pstr, p[i].len);
        break;
    }

    if (p[i].used == p[i].size)
      ++nfull;
  }
#endif
  printf("Read %d unique subnets.\n", num);
  printf("%d are prefixes of %d base subnets using %lu kB memory.\n",
         nprefixes, nbases, ((nprefixes + nbases) * sizeof(lct_subnet_t))/1024);
  printf("%d prefixes are fully allocated to subprefixes.\n", nfull);

  return 0;
}
