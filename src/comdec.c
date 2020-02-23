#include "const.h"
#include "sequitur.h"
#include "debug.h"
#include "extras.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

int rule_delimeter = 0;
int prev_rule_delimeter = 1;
unsigned char prevChanged = 0;
int num_bytes = 0;
int count_bytes = 0;
int decompressBytes = 0;

/**
 * Compares two strings and returns whether they are equals or not.
 * 
 * @param *s1 The first string value that needs to be compared.
 * 
 * @param *s2 The second string value that needs to be compared
 * 
 * @return An integer that returns 1 if the strings are equal and returns a 0 if they are not.
 */
int compareStr(char *s1, char *s2)
{
    while (*s1 == *s2 && *s1 != '\0')
    {
        s1 += 1;
        s2 += 1;
    }

    if (*s1 == '\0' && *s2 == '\0')
        return 1;

    return 0;
}

/**
 * Converts a string to an integer
 * 
 * @param *num The string that needs to be converted to an integer
 * 
 * @return The integer equivalent of the string if it only consists of integers.
 */
int convertStrToInt(char *num)
{
    if (compareStr("", num))
        return -1;

    int ans = 0;
    char currentChar;

    while (*num != '\0')
    {
        currentChar = *num;
        switch (*num)
        {
        case '0':
        {
            ans *= 10;
            break;
        }
        default:
        {
            if (atoi(&currentChar) == 0)
                return -1;

            ans *= 10;
            ans += atoi(&currentChar);
        }
        }
        num += 1;
    }
    return ans;
}

/**
 * Checks how many bytes need to be read and returns the number.
 * 
 * @param c The unsigned char that needs to be checked.
 * 
 * @return The number of bytes that need to be read after the current character
 */
int bitChecker(unsigned int c)
{

    if ((c & 0x80) == 0)
        return 1;
    else if ((c & 0xE0) == 0xC0)
        return 2;
    else if ((c & 0xF0) == 0xE0)
        return 3;
    else if ((c & 0xF8) == 0xF0)
        return 4;
    return 1;
}

/**
 * Checks how many bytes need to be read and returns the number.
 * 
 * @param c The unsigned char that needs to be checked.
 */
int charChecker(unsigned char c, FILE *output)
{
    if (c == 0x85)
    { // At the end of every rule
        rule_delimeter = 1;
        prev_rule_delimeter = rule_delimeter;
        return 0;
    }
    else if (c == 0x82 && prevChanged == 0x84)
    { // At the end of transmission
        decompressCFG(main_rule, output);
        fflush(output);
        return 0;
    }
    else if (c == 0x83 && prevChanged == 0x84)
    { // At the end of every block
        prev_rule_delimeter = 1;
        decompressCFG(main_rule, output);
        init_rules();
        init_symbols();
        fflush(output);
        return 0;
    }
    else if (c == 0x83 && prevChanged == 0x81)
    { // At the start of transmission
        return 0;
    }
    else if (c == 0x81 || c == 0x84)
        return 0;

    rule_delimeter = 0;
    return 1;
}

int getUTF8(char currChanged, FILE *in)
{
    char charRead;
    unsigned int value;
    int bitLength = bitChecker(currChanged);

    if (bitLength == 4)
        value = currChanged & 0x7;
    else if (bitLength == 3)
        value = currChanged & 0xF;
    else if (bitLength == 2)
        value = currChanged & 0x1F;
    else
        value = currChanged;

    bitLength--;
    while (bitLength > 0)
    {
        charRead = fgetc(in);
        value = (value << 6);
        value |= (charRead & 0x3F);
        bitLength--;
    }

    return value;
}

/**
 * Main compression function.
 * Reads a sequence of bytes from a specified input stream, segments the
 * input data into blocks of a specified maximum number of bytes,
 * uses the Sequitur algorithm to compress each block of input to a list
 * of rules, and outputs the resulting compressed data transmission to a
 * specified output stream in the format detailed in the header files and
 * assignment handout.  The output stream is flushed once the transmission
 * is complete.
 *
 * The maximum number of bytes of uncompressed data represented by each
 * block of the compressed transmission is limited to the specified value
 * "bsize".  Each compressed block except for the last one represents exactly
 * "bsize" bytes of uncompressed data and the last compressed block represents
 * at most "bsize" bytes.
 *
 * @param in  The stream from which input is to be read.
 * @param out  The stream to which the block is to be written.
 * @param bsize  The maximum number of bytes read per block.
 * @return  The number of bytes written, in case of success,
 * otherwise EOF.
 */
int compress(FILE *in, FILE *out, int bsize)
{
    int bsizeCounter = 0;

    init_rules();
    init_symbols();
    init_digram_hash();

    add_rule(new_rule(next_nonterminal_value++));

    unsigned char charRead;
    unsigned int value;

    fputc(0x81, out);
    fputc(0x83, out);

    while (!feof(in))
    {
        charRead = (unsigned char)fgetc(in);
        if (charRead == 255)
            break;
        bsizeCounter++;
        SYMBOL *symbol_to_add = new_symbol(charRead, NULL);

        if (bsizeCounter == bsize)
        {
            print_main_rule();

            printEncodedBlock(out);
            init_symbols();
            init_rules();
            init_digram_hash();
            add_rule(new_rule(FIRST_NONTERMINAL));
            next_nonterminal_value++;
            count_bytes += bsizeCounter;
            bsizeCounter = 0;
            fputc(0x84, out);
            fputc(0x83, out);
        }

        insert_after(main_rule->prev, symbol_to_add);
        check_digram(main_rule->prev->prev);
    }
    print_main_rule();
    printEncodedBlock(out);
    fputc(0x82, out);
    if (fputc(0x84, out) == -1)
    {
        fflush(out);
        return -1;
    }
    return count_bytes;
}

void decompressCFG(SYMBOL *currRule, FILE *output)
{
    SYMBOL *head = currRule;
    SYMBOL *pointer = currRule->next;

    while (pointer != head)
    {
        if (pointer->value < FIRST_NONTERMINAL)
        {
            decompressBytes = fputc(pointer->value, output);
            count_bytes++;
        }
        else
        {
            decompressCFG(*(rule_map + pointer->value), output);
        }
        pointer = pointer->next;
    }
}

/**
 * Main decompression function.
 * Reads a compressed data transmission from an input stream, expands it,
 * and writes the resulting decompressed data to an output stream.
 * The output stream is flushed once writing is complete.`
 *
 * @param in  The stream from which the compressed block is to be read.
 * @param out  The stream to which the uncompressed data is to be written.
 * @return  The number of bytes written, in case of success, otherwise EOF.
 */
int decompress(FILE *in, FILE *out)
{
    count_bytes = 0;
    char charRead;
    unsigned int value, currChanged;
    char nextChar;
    SYMBOL *currentRuleBeingRead;

    init_rules();
    init_symbols();

    while (!feof(in))
    {
        charRead = fgetc(in);
        prevChanged = currChanged;
        currChanged = (unsigned int)charRead & 0xFF;
        if (charChecker(currChanged, out) == 1)
        {
            value = getUTF8(currChanged, in);

            if (prev_rule_delimeter == 1)
            { // Create a new rule
                currentRuleBeingRead = NULL;
                prev_rule_delimeter = 0;
                currentRuleBeingRead = new_rule(value);
                add_rule(currentRuleBeingRead);
                *(rule_map + value) = currentRuleBeingRead;
            }
            else
            { // Add to an existing rule
                if (currentRuleBeingRead == NULL)
                {
                    currentRuleBeingRead = new_rule(value);
                    add_rule(currentRuleBeingRead);
                }
                else
                    add_symbol(new_symbol(value, NULL), currentRuleBeingRead);
            }
        }
    }
    if (decompressBytes == -1)
        return EOF;
    else
        return decompressBytes;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    global_options = 0;
    // If there are no flags
    if (argc < 2)
        return -1;

    // If -h is present
    if (compareStr(*(argv + 1), "-h"))
    {
        global_options = 1;
        return 0;
    }

    // If -c is present
    if (compareStr(*(argv + 1), "-c"))
    {
        if (argc == 2)
        {
            global_options = 1024;
            global_options = (global_options << 16) + 2;
            return 0;
        }
        else if (argc == 4 && compareStr(*(argv + 2), "-b"))
        {
            int blockSize = convertStrToInt(*(argv + 3));
            if (blockSize < 1 || blockSize > 1024)
                return -1;
            global_options = blockSize;
            global_options = (global_options << 16) + 2;
            return 0;
        }
        else
            return -1;
    }

    // If -d is present
    if (compareStr(*(argv + 1), "-d"))
    {
        if (argc > 2)
            return -1;

        global_options = 4;
        return 0;
    }

    return -1;
}

void printEncodedBlock(FILE *output)
{
    debug("Entered printEncodedBock");
    if (main_rule != NULL)
    {
        SYMBOL *rule_cursor = main_rule;
        do
        {
            SYMBOL *temp = rule_cursor;
            do
            {
                fputc(temp->value, output);
                temp = temp->next;
                fputc(0x85, output);
            } while (temp != rule_cursor);

            rule_cursor = rule_cursor->nextr;
        } while (rule_cursor != main_rule);
    }

    fflush(output);
}