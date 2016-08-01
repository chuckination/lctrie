#ifndef __LC_TRIE_H__
#define __LC_TRIE_H__
// begin #ifndef guard

#include <stdlib.h>
#include <stdint.h>

#include "lctrie_ip.h"

// Since a Trie is a radix tree, the number of
// operations to lookup a key in a tree is dependent
// on the depth of the tree.  The more bits allow for
// a greater number of child nodes per node and more
// bit comparisons in a single traversal of the trie,
// or a trie-versal. (lame programmer joke)
//
// Level compressed (LC) Trie
// a multibit trie implementation with a densely
// packed root node for less densely packed child
// sub-tries for minimal tree depth.
//
// This LC Trie is an array indexed implementation
// that attempts to make use of byte aligned values
// to reduce the amount of bit shifting operations
// in the cpu for using the data structures.
//
// Other reference implementations make use of bit packing
// to conserve memory at the expense of CPU operations.
// We will attempt to only make use of bit operations when
// actually manipulating the key data itself.
//
// A trie branch node has n children which are indexed by
// the search key's n-bit values after skipping m-bits from
// the current search position where n is the node's branch value
// and m is the node's skip value.
//
// Leaf nodes are indicated by a branch value of zero, and branch nodes have
// a non-zero branch value.  This removes the need to store a node type in
// another field.
//
// Since each node is only the size of two pointers on 32 and 64 bit systems
// this should be a relatively small data structure even for large network
// sets (such as the union of RFC1918 and the latest ASN ipv4 subnet assignments
// freshly splurped up from the internet).
typedef struct lct_node {
  uint8_t branch;
  uint8_t skip;
  union {
    struct lct_node *node;
    lct_subnet_t *data;
  } ptr;
} lct_node_t;

// The size of the the trie is going to be
// 2 * number of prefixes stored with nulls
// sparsely mixed amongst the trie levels.

// LC Trie data structure
// size - number of nodes in the trie
// trie - the root of the trie
typedef struct lct {
  size_t ncount;  // number of trie nodes, will always be <= 2 * pcount
  size_t pcount;  // number of trie leaves, will always be smaller than ncount
  lct_node_t *root;
} lct_t;

// lifecycle functions
//
// we store pointers to the subnet passed in here, so the subnet array must
// remain static during the lifetime of the trie.  if the array must be changed,
// free the trie before doing so and recreate it afterwards.  This doesn't bode
// well for a large number of dynamic updates, but keeping updates to a minimum
// and potentially double buffering the data can reduce latency for these
// events.
void lct_build(lct_t *trie, lct_subnet_t *bases, lct_subnet_t *prefixes, size_t pcount);
void lct_free(lct_t *trie);

// search function
// return the IP subnet corresponding to the element,
// otherwise return NULL if not found
lct_subnet_t *lct_find(lct_t *trie, uint32_t key);

// end #ifndef guard
#endif
