#include "sequitur.h"
#include "const.h"

/**
 * Extra variables that allow me to function properly.
 */
extern int num_recycled_symbols;
extern int rule_delimeter;
extern int prev_rule_delimeter;
extern unsigned char prevChanged;
extern int numBlocks;
extern int count_bytes;
extern SYMBOL last_recycled_symbol;

/**
 * Decompression methods to help me do the extra work in the program.
 */ 
int convertStrToInt(char *num);
int bitChecker(unsigned int c);
int charChecker(unsigned char C, FILE *output);
int compareStr(char *s1, char *s2);
int getUTF8(char changed, FILE *in);
void add_symbol(SYMBOL *sym, SYMBOL *rule);
void print_rule_map();
void print_main_rule();
void print_single_rule(SYMBOL *curr_rule);
void decompressCFG(SYMBOL *currRule, FILE *output);
int check_digram_equality(SYMBOL *s1, SYMBOL *s2);

/**
 * Compression Methods that do the extra work for me
 */
int check_equal_digram_value(SYMBOL *s1, SYMBOL *s2);
int check_digram_equality_pointer(SYMBOL *s1, SYMBOL *s2);
void printEncodedBlock(FILE *output);