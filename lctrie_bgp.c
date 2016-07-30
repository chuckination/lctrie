#include "lctrie_bgp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>

int
read_prefix_table(char *filename,
                  lct_bgp_prefix_t prefix[],
                  size_t prefix_size) {
	int num = 0;
	FILE *infile;
	char str[4];
	int len, asn;

	// open the file for reading
	if (!(infile = fopen(filename, "r"))) {
		fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		return -1;
	}

	// TODO clean up this ugliness
	char addr_str[INET_ADDRSTRLEN];
	while (EOF != fscanf(infile, "%hhu.%hhu.%hhu.%hhu/%d\t%d",
											 &str[0], &str[1], &str[2], &str[3], &len, &asn)) {
		// TODO eliminate reconverting the dotted quad bytes into another string
		snprintf(addr_str, sizeof(addr_str), "%hhu.%hhu.%hhu.%hhu",
				     str[0], str[1], str[2], str[3]);

		// convert the string IP address to an IP address
		if (0 >= inet_pton(AF_INET, addr_str, &(prefix[num].prefix))) {
			fprintf(stderr, "invalid address %s: %s\n", addr_str, strerror(errno));
			continue;
		}
		prefix[num].len = len;
		prefix[num].asn = asn;
		num++;
	}

	return num;
}

int prefix_cmp(lct_bgp_prefix_t **i, lct_bgp_prefix_t **j) {
	if ((*i)->prefix < (*j)->prefix)
		return -1;
	else if ((*j)->prefix > (*j)->prefix)
		return 1;
	else if ((*i)->len < (*j)->len)
		return -1;
	else if ((*i)->len > (*j)->len)
		return 1;
	else
		return 0;
}

int
read_asn_table(char *filename,
               lct_bgp_asn_t prefix[],
               size_t prefix_size) {
  return -1;
}
