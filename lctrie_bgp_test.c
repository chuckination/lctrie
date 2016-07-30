#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>

#include "lctrie_bgp.h"

#define BGP_MAX_ENTRIES 4000000

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <BGP ASN Subnets> <BGP ASN Assignments>\n", basename(argv[0]));
    exit(EXIT_FAILURE);
  }

	int num;
	lct_bgp_prefix_t *prefixes;
	if (!(prefixes = (lct_bgp_prefix_t *)malloc(BGP_MAX_ENTRIES * sizeof(lct_bgp_prefix_t)))) {
		fprintf(stderr, "Could not allocate prefix array\n");
		exit(EXIT_FAILURE);
	}

	printf("Reading prefixes from %s...\n", argv[1]);
	if (0 > (num = read_prefix_table(argv[1], prefixes, BGP_MAX_ENTRIES))) {
		fprintf(stderr, "could not read prefix file \"%s\"\n", argv[1]);
		return num;
	}

	char inet_p[INET_ADDRSTRLEN];
	for (int i = 0; i < num; i++) {
		if (!inet_ntop(AF_INET, &(prefixes[i].prefix), inet_p, sizeof(inet_p))) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			continue;
		}
		//printf("Prefix %d/%d for ASN %d\n", prefixes[i].prefix, prefixes[i].len, prefixes[i].asn);
		printf("Prefix %s/%d for ASN %d\n", inet_p, prefixes[i].len, prefixes[i].asn);
	}
	printf("Read %d prefixes.\n", num);

  return 0;
}
