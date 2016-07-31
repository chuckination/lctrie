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

  if (argc != 3) {
    fprintf(stderr, "usage: %s <BGP ASN Subnets> <BGP ASN Assignments>\n", basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  if (!(p = (lct_subnet_t *)malloc(BGP_MAX_ENTRIES * sizeof(lct_subnet_t)))) {
    fprintf(stderr, "Could not allocate prefix input buffer\n");
    exit(EXIT_FAILURE);
  }
  memset(p, 0, BGP_MAX_ENTRIES * sizeof(lct_subnet_t));

#if LCT_IP_READ_PREFIXES_FILES
  // read in the ASN prefixes
  printf("Reading prefixes from %s...\n", argv[1]);
  if (0 > (num = read_prefix_table(argv[1], p, BGP_MAX_ENTRIES))) {
    fprintf(stderr, "could not read prefix file \"%s\"\n", argv[1]);
    return num;
  }
#endif

  // fill up the rest of the array with reserved IP subnets
  num += init_reserved_subnets(&(p[num]), BGP_MAX_ENTRIES - num);

  // shrink the buffer down to its actual size
  p = realloc(p, num * sizeof(lct_subnet_t));

  // sort the resulting array
  qsort(p, num, sizeof(lct_subnet_t), subnet_cmp);

#if LCT_IP_DISPLAY_PREFIXES
  char pstr[INET_ADDRSTRLEN];
  for (int i = 0; i < num; i++) {
    uint32_t prefix = htonl(p[i].prefix);
    if (!inet_ntop(AF_INET, &(prefix), pstr, sizeof(pstr))) {
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      continue;
    }

    switch (p[i].info.type) {
      case IP_SUBNET_BGP:
        printf("BGP prefix %s/%d for ASN %d\n", pstr, p[i].len, p[i].info.bgp.asn);
        break;

      case IP_SUBNET_PRIVATE:
        printf("Private class %c subnet for %s/%d\n", p[i].info.priv.class, pstr, p[i].len);
        break;

      case IP_SUBNET_LINKLOCAL:
        printf("Link local subnet for %s/%d\n", pstr, p[i].len);
        break;

      case IP_SUBNET_MULTICAST:
        printf("Multicast subnet for %s/%d\n", pstr, p[i].len);
        break;

      case IP_SUBNET_BROADCAST:
        printf("Broadcast subnet for %s/%d\n", pstr, p[i].len);
        break;

      case IP_SUBNET_LOOPBACK:
        printf("Loopback subnet for %s/%d\n", pstr, p[i].len);
        break;

      case IP_SUBNET_RESERVED:
        printf("Reserved subnet for %s/%d, %s\n", pstr, p[i].len, p[i].info.rsv.desc);
        break;

      case IP_SUBNET_BOGON:
        printf("Bogon subnet for %s/%d\n", pstr, p[i].len);
        break;


      default:
        printf("Invalid prefix type for %s/%d\n", pstr, p[i].len);
        break;
    }
  }
#endif
  printf("Read %d subnets into %lu kB (%lu B/subnet).\n", num, (num * sizeof(lct_subnet_t)) / 1024, sizeof(lct_subnet_t));

  return 0;
}
