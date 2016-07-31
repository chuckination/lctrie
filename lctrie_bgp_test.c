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
	lct_subnet_t *prefixes, *p;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <BGP ASN Subnets> <BGP ASN Assignments>\n", basename(argv[0]));
    exit(EXIT_FAILURE);
  }

	if (!(p = (lct_subnet_t *)malloc(BGP_MAX_ENTRIES * sizeof(lct_subnet_t)))) {
		fprintf(stderr, "Could not allocate prefix input buffer\n");
		exit(EXIT_FAILURE);
	}
  memset(p, 0, BGP_MAX_ENTRIES * sizeof(lct_subnet_t));

	// read in the ASN prefixes
	printf("Reading prefixes from %s...\n", argv[1]);
	if (0 > (num = read_prefix_table(argv[1], p, BGP_MAX_ENTRIES))) {
		fprintf(stderr, "could not read prefix file \"%s\"\n", argv[1]);
		return num;
	}
	
	// fill up the rest of the array with reserved IP subnets
	num += init_reserved_subnets(&(p[num]), BGP_MAX_ENTRIES - num);	

  // fill up the input buffer now that we know how many prefixes we really need
	if (!(prefixes = (lct_subnet_t *)malloc(num * sizeof(lct_subnet_t)))) {
		fprintf(stderr, "Could not allocate prefix storage array\n");
		exit(EXIT_FAILURE);
	}
  memcpy(prefixes, p, num * sizeof(lct_subnet_t));
  free(p);
  p = prefixes;

	// sort the resulting array
	qsort(prefixes, num, sizeof(lct_subnet_t), subnet_cmp);

#if 0
  char inet_p[INET_ADDRSTRLEN];
	for (int i = 0; i < num; i++) {
		if (!inet_ntop(AF_INET, &(p[i].prefix), inet_p, sizeof(inet_p))) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			continue;
		}

		switch (p[i].info.type) {
			case IP_SUBNET_BGP:
				printf("BGP prefix %s/%d for ASN %d\n", inet_p, p[i].len, p[i].info.bgp.asn);
				break;

			case IP_SUBNET_PRIVATE:
				printf("Private class %c subnet for %s/%d\n", p[i].info.priv.class, inet_p, p[i].len);
				break;

			case IP_SUBNET_LINKLOCAL:
				printf("Link local subnet for %s/%d\n", inet_p, p[i].len);
				break;

			default:
				printf("Invalid prefix type for %s/%d\n", inet_p, p[i].len);
				break;
		}
	}
#endif
	printf("Read %d prefixes.\n", num);

  return 0;
}
