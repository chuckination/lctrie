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
#define LCT_IP_DISPLAY_PREFIXES     0

int main(int argc, char *argv[]) {
  int num = 0;
  lct_subnet_t *p;
  int nprefixes = 0, nbases = 0;
  lct_subnet_t *prefixes, *bases;
  lct_subnet_t *temp;

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

  // sort the resulting array
  qsort(p, num, sizeof(lct_subnet_t), subnet_cmp);

  // remove duplicates
  char pstr[INET_ADDRSTRLEN];
  uint32_t prefix;
#if LCT_IP_DISPLAY_PREFIXES
  char pstr2[INET_ADDRSTRLEN];
  uint32_t prefix2;
#endif
  printf("Removing duplicates...\n");
  int ndup = 0;
  for (int i = 0, j = 1; j < num; ++i, ++j) {
    // we have a duplicate!
    if (!subnet_cmp(&p[i], &p[j])) {
      prefix = htonl(p[i].addr);
      if (!inet_ntop(AF_INET, &(prefix), pstr, sizeof(pstr)))
        fprintf(stderr, "ERROR: %s\n", strerror(errno));

      printf("Subnet %s/%d type %d duplicates another of type %d\n",
             pstr, p[i].len, p[i].info.type, p[j].info.type);

      // assume that the later defined subnet is the desired one,
      // allowing for overrides with the risk of overriding reserved
      // subnets
      memmove(&p[i], &p[j], (num - j) * sizeof(lct_subnet_t));
      --num;
      ++ndup;
    }
  }

  // shrink the buffer down to its actual size and split into prefixes and bases
  p = realloc(p, num * sizeof(lct_subnet_t));
  printf("%d duplicates removed\n", ndup);

  if (!(prefixes = (lct_subnet_t *)malloc(num * sizeof(lct_subnet_t)))) {
    fprintf(stderr, "Could not allocate prefixes output buffer\n");
    free(p);
    exit(EXIT_FAILURE);
  }
  if (!(bases = (lct_subnet_t *)malloc(num * sizeof(lct_subnet_t)))) {
    fprintf(stderr, "Could not allocate prefixes output buffer\n");
    free(p);
    exit(EXIT_FAILURE);
  }

  // go through and determine which subnets are prefixes of other subnets
  for (int i = 0; i < num; ++i) {
    int j = i + 1;  // fake out a psuedo second iterator
    if (i < (num - 1) && subnet_isprefix(&p[i], &p[j])) {
#if LCT_IP_DISPLAY_PREFIXES
      prefix = htonl(p[i].addr);
      prefix2 = htonl(p[j].addr);
      if (!inet_ntop(AF_INET, &(prefix), pstr, sizeof(pstr)))
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
      if (!inet_ntop(AF_INET, &(prefix2), pstr2, sizeof(pstr2)))
        fprintf(stderr, "ERROR: %s\n", strerror(errno));

      printf("Subnet %s/%d is a prefix of subnet %s/%d\n",
             pstr, p[i].len, pstr2, p[j].len);
#endif

      // add it to the prefix array
      temp = &prefixes[nprefixes++];
      memcpy(temp, &p[i], sizeof(lct_subnet_t));
      p[j].prefix = temp;

      for (int k = j + 1; k < num && subnet_isprefix(&p[i], &p[k]); ++k) {
#if LCT_IP_DISPLAY_PREFIXES
        prefix2 = htonl(p[k].addr);
        if (!inet_ntop(AF_INET, &(prefix2), pstr2, sizeof(pstr2)))
          fprintf(stderr, "ERROR: %s\n", strerror(errno));

        printf("Subnet %s/%d is also a prefix of subnet %s/%d\n",
               pstr, p[i].len, pstr2, p[k].len);
#endif
        p[k].prefix = temp;
      }
    }
    else {
      // add it to the base array
      temp = &bases[nbases++];
      memcpy(temp, &p[i], sizeof(lct_subnet_t));
    }
  }

  // resize the base and prefix arrays
  prefixes = realloc(prefixes, nprefixes * sizeof(lct_subnet_t));
  bases = realloc(bases, nbases * sizeof(lct_subnet_t));

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
  printf("Read %d unique subnets.\n", num);
  printf("%d are prefixes of %d base subnets using %lu kB memory.\n",
         nprefixes, nbases,
         2 * ((nprefixes + nbases) * sizeof(lct_subnet_t))/1024);

  return 0;
}
