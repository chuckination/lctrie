This project provides a level compressed trie C library
for the purposes of matching IP addresses to ASNs via
informational files published via APNIC regarding
IP prefixes mapped to ASNs and ASNs mapped to owners.

Another implementation is in the linux kernel inside
fib_trie.c which has code for adding/removing entries from
the table, so that may become the reference implementation
over the Dr. Dobbs example source code.

For our purposes, it should be sufficient to dump the trie
and start from scratch again if we want to rebuild the trie.
This will correspond to a front/back buffer should this need
to be wired into a performance critical asynchronous system.
