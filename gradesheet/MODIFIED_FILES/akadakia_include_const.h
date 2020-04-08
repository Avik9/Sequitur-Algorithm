--- hw1/dev_repo/solution/include/const.h

+++ hw1/repos/akadakia/hw1/include/const.h

@@ -56,7 +56,7 @@

 /*

  * Array, used during decompression, that maps symbol values to nonterminal symbols.

  */

-SYMBOL *rule_map[SYMBOL_VALUE_MAX];

+SYMBOL *rule_map[MAX_SYMBOLS];

 

 /*

  * Below this line are prototypes for functions that MUST occur in your program.
