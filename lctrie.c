#include "lctrie.h"


// since the build algorithm is recursive, we'll pass this API entry point
// into an interior build function
void lct_build(lct_t *trie, lct_subnet_t *bases, lct_subnet_t *prefixes, size_t pcount) {
  // user is responsible for the outer struct,
  // and we're responsible for the interior memory

  // TODO implement me!
}

void lct_free(lct_t *trie) {
  if (!trie)
    return;

  free(trie->root);
  trie->root = NULL;
  trie->ncount = 0;
  trie->pcount = 0;
}

lct_subnet_t *lct_find(lct_t *trie, uint32_t key) {
  // TODO implement me!
  return NULL;
}
