#ifndef __LC_TRIE_H__
#define __LC_TRIE_H__
// begin #ifndef guard

// begin C++ extern guard
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* The trie is represented by an array and each node in
   the trie is compactly represented using only 32 bits:
   5 + 5 + 22 = branch + skip + adr */
typedef uint32_t lct_node_t;


// end C++ extern guard
#ifdef __cplusplus
}
#endif

// end #ifndef guard
#endif
