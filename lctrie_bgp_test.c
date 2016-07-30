#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>

#include "lctrie_bgp.h"

#define BGP_MAX_ENTRIES 4000000

int main(int argc, char *argv[]) {
	int num;
	lct_bgp_prefix_t *prefixes, *p;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <BGP ASN Subnets> <BGP ASN Assignments>\n", basename(argv[0]));
    exit(EXIT_FAILURE);
  }

	if (!(p = (lct_bgp_prefix_t *)malloc(BGP_MAX_ENTRIES * sizeof(lct_bgp_prefix_t)))) {
		fprintf(stderr, "Could not allocate prefix input buffer\n");
		exit(EXIT_FAILURE);
	}
  memset(p, 0, BGP_MAX_ENTRIES * sizeof(lct_bgp_prefix_t));

	printf("Reading prefixes from %s...\n", argv[1]);
	if (0 > (num = read_prefix_table(argv[1], p, BGP_MAX_ENTRIES))) {
		fprintf(stderr, "could not read prefix file \"%s\"\n", argv[1]);
		return num;
	}

  // fill up the input buffer now that we know how many prefixes we really need
	if (!(prefixes = (lct_bgp_prefix_t *)malloc(num * sizeof(lct_bgp_prefix_t)))) {
		fprintf(stderr, "Could not allocate prefix storage array\n");
		exit(EXIT_FAILURE);
	}
  memcpy(prefixes, p, num * sizeof(lct_bgp_prefix_t));
  free(p);

#if 0
  p = prefixes;
  char inet_p[INET_ADDRSTRLEN];
	for (int i = 0; i < num; i++) {
		if (!inet_ntop(AF_INET, &(p[i].prefix), inet_p, sizeof(inet_p))) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			continue;
		}
		printf("Prefix %s/%d for ASN %d\n", inet_p, p[i].len, p[i].asn);
	}
	printf("Read %d prefixes.\n", num);
#endif

  return 0;
}
