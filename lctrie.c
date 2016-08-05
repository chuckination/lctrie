#include "lctrie.h"

#include <stdio.h>

static void compute_skip(lct_t *trie, uint32_t prefix, uint32_t first,
                         uint32_t count, uint32_t *skip, uint32_t *newprefix) {
  // there is no skip factor on the root node
  if ((prefix == 0) && (first == 0)) {
    *skip = 0;
    return;
  }

  // TODO finish me!
}

static void compute_branch(lct_t *trie, uint32_t prefix, uint32_t first,
                           uint32_t count, uint32_t *branch, uint32_t *newprefix) {
  // branch factor results in 1 << branch trie subnodes

  // the first two levels are fixed no matter what.
  // always use a branching factor of 4 bits at the root.
  // multicast and reserved spaces are /4 networks and
  // we'll need to check for those.
  if ((prefix == 0) && (first == 0)) {
    *branch = 2;
    return;
  }
 
  // always use a branch factor of 2 for two element arrays
  if (count == 2) {
    *branch = 2;
    return;
  }

  // TODO finish me!
}

static void build_inner(lct_t *trie, uint32_t prefix, uint32_t first, uint32_t count, uint32_t pos) {
  // TODO finish me!
}

// since the build algorithm is recursive, we'll pass this API entry point
// into an interior build function
int lct_build(lct_t *trie, lct_subnet_t *subnets, uint32_t size) {
  if (!trie)
    return -1;

  // user is responsible for the outer struct,
  // and we're responsible for the interior memory
  trie->nets = subnets;

  // bases will never be more than size, but we will need to
  // shrink it back down after it's allocated
  trie->bases = (uint32_t *) malloc(size * sizeof(uint32_t));

  // allocate and count the bases
  trie->bcount = 0;
  if (!trie) {
    fprintf(stderr, "ERROR: failed to allocate trie bases index buffer\n");
    return -1;
  }

  for (int i = 0; i < size; ++i) {
    if (IP_PREFIX_FULL == subnets[i].type) {
      // save off the base's index in the subnet array
      // and increment the bases counter
      trie->bases[trie->bcount++] = i;
    }
  }

  // reallocate the base index buffer back down to the actual size.
  trie->bases = (uint32_t *) realloc(trie->bases, trie->bcount * sizeof(uint32_t));

  // give a 2MB buffer, and we'll shrink it down once we've built the trie
  trie->root = (lct_node_t *) malloc((size + 2000000) * sizeof(lct_node_t));
  if (!trie->root) {
    free(trie->bases);
    fprintf(stderr, "ERROR: failed to allocate trie node buffer\n");
    return -1;
  }

  // hande off to the inner recursive function
  trie->ncount = 0; // determine trie node count later
  build_inner(trie, 0x00000000, 0, trie->bcount, 0);

  // shrink down the trie node array to its actual size
  trie->root = (lct_node_t *) realloc(trie->root, trie->ncount * sizeof(lct_node_t));

  // TODO anything else before we return from this?

  return 0;
}

void lct_free(lct_t *trie) {
  if (!trie)
    return;

  // don't free the external subnet array.
  // that's under outside control.
  free(trie->bases);
  trie->bases = NULL;
  trie->root = NULL;
  trie->ncount = 0;
  trie->bcount = 0;
}

lct_subnet_t *lct_find(lct_t *trie, uint32_t key) {
  // TODO implement me!
  return NULL;
}
