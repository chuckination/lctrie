#include "lctrie_bgp.h"

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <BGP ASN Subnets> <BGP ASN Assignments>\n", basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  return 0;
}
