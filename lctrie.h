#ifndef __LC_TRIE_H__
#define __LC_TRIE_H__
// begin #ifndef guard

#include <stdlib.h>
#include <stdint.h>

// Since a Trie is a radix tree, the number of
// operations to lookup a key in a tree is dependent
// on the depth of the tree.  The more bits allow for
// a greater number of child nodes per node and more
// bit comparisons in a single traversal of the trie,
// or a trie-versal. (lame programmer joke)

// Level compressed (LC) Trie
// a multibit trie implementation with a densely
// packed root node for less densely packed child
// sub-tries for minimal tree depth.

// This LC Trie is an array indexed implementation
// that attempts to make use of byte aligned values
// to reduce the amount of bit shifting operations
// in the cpu for using the data structures.

// Other reference implementations make use of bit packing
// to conserve memory at the expense of CPU operations.
// We will attempt to only make use of bit operations when
// actually manipulating the key data itself.

// A trie branche node has n children which are indexed by
// the search key's bit values indicated by the values in the trie node.
//
// Since each node is only the size of three pointers on 32 bit systems
// and two pointers on 64 bit systems, this should be a relatively small
// data structure even for large network sets (such as the union of RFC1918
// and the latest ASN ipv4 subnet assignments freshly splurped up from the
// internet).
typedef struct lct_node {
  uint16_t branch;
  uint16_t skip;
  uint32_t ptr;
  uint32_t key;
  void *data;
} lct_node_t;

// The size of the the trie is going to be
// 2 * number of prefixes stored with nulls
// sparsely mixed amongst the trie levels.

// LC Trie data structure
// size - number of nodes in the trie
// trie - the root of the trie
typedef struct lct {
  size_t size;
  lct_node_t *trie;
} lct_t;

// TODO fix these up

// lifecycle functions
lct_t *lct_build();
void lct_free(lct_t *trie);

// search function
// returns NULL
void **lct_free(lct_t *trie);

// end #ifndef guard
#endif
