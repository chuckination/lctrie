#include "lctrie_bgp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <arpa/inet.h>

#include <pcre.h>

int
read_prefix_table(char *filename,
                  lct_bgp_prefix_t prefix[],
                  size_t prefix_size) {
	int num = 0;
	FILE *infile;
  char *line = NULL;
  size_t line_len = 0;

  // naively format for prefix table file.  IP addresses will
  // be validated as they're parsed by inet_pton()
  pcre *re;
  char *pattern = "^((\\d{1,3}\\.){3}\\d{1,3})\\/(\\d{1,2})\\t(\\d+)$";
  const char *error;
  int erroffset;
  int rc;
#define BGP_PREFIX_OVECCOUNT 3 * 5 // we'll have 5 substring matches
  int ovector[BGP_PREFIX_OVECCOUNT];

  // the actual line field input buffer used for input validation
	char input[INET_ADDRSTRLEN];
  char *substr_start;
  int substr_len;

	// open the file for reading
	if (!(infile = fopen(filename, "r"))) {
		fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		return -1;
	}

  re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
  if (!re) {
    fprintf(stderr, "PCRE compilation failed at offset: %d: %s\n",
            erroffset, error);
    return -1;
  }

  // using a mix of PCRE, inet_pton, and strtoul,
  // validate and parse each line of input
  while (-1 != getline(&line, &line_len, infile)) {
    // clip off the trailing newline character
    line[strcspn(line, "\n")] = 0;

    // match against the semi-naive PCRE regex as
    // first pass validation and input string retrieval
    rc = pcre_exec(re,
                   0,
                   line,
                   strlen(line),
                   0,
                   0,
                   ovector,
                   BGP_PREFIX_OVECCOUNT);
    if (rc < 0) {
      switch (rc) {
        case PCRE_ERROR_NOMATCH:
          fprintf(stderr, "invalid line: %s\n", line);
          break;

        default:
          fprintf(stderr, "Matching error %d on line: %s\n", rc, line);
          break;
      }
      continue;
    }

    // validate and extract IP prefix, index 1 in the PCRE matches
    substr_start = line + ovector[2*1];
    substr_len = ovector[2*1 + 1] - ovector[2*1];
    snprintf(input, sizeof(input), "%.*s", substr_len, substr_start);
    if (!inet_pton(AF_INET, input, &(prefix[num].prefix))) {
      fprintf(stderr, "ERROR: %s is not a valid IP address: %s\n", input, strerror(errno));
      continue;
    }

    // validate and extract prefix length, index 3 in the PCRE matches
    substr_start = line + ovector[2*3];
    substr_len = ovector[2*3 + 1] - ovector[2*3];
    snprintf(input, sizeof(input), "%.*s", substr_len, substr_start);
    if (ULONG_MAX == (prefix[num].len = strtoul(input, NULL, 10))) {
      fprintf(stderr, "ERROR: %s is not a valid integer: %s\n", input, strerror(errno));
      continue;
    }
    if ((prefix[num].len == 0) || (prefix[num].len > 32)) {
      fprintf(stderr, "ERROR: %d is not a valid prefix length\n", prefix[num].len);
      continue;
    }

    // validate and extract ASN, index 4 in the PCRE matches
    substr_start = line + ovector[2*4];
    substr_len = ovector[2*4 + 1] - ovector[2*4];
    snprintf(input, sizeof(input), "%.*s", substr_len, substr_start);
    if (ULONG_MAX == (prefix[num].asn = strtoul(input, NULL, 10))) {
      fprintf(stderr, "ERROR: %s is not a valid integer: %s\n", input, strerror(errno));
      continue;
    }

		num++;
	}

  free(line);
  pcre_free(re);
  fclose(infile);

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
