#include "const.h"
#include "sequitur.h"

/*
 * Symbol management.
 *
 * The functions here manage a statically allocated array of SYMBOL structures,
 * together with a stack of "recycled" symbols.
 */

/*
 * Initialization of this global variable that could not be performed in the header file.
 */
int next_nonterminal_value = FIRST_NONTERMINAL;
SYMBOL *last_recycled_symbol;

/**
 * Initialize the symbols module.
 * Frees all symbols, setting num_symbols to 0, and resets next_nonterminal_value
 * to FIRST_NONTERMINAL;
 */
void init_symbols(void)
{
    next_nonterminal_value = FIRST_NONTERMINAL;
    num_symbols = 0;
    last_recycled_symbol = NULL;
}

/**
 * Get a new symbol.
 *
 * @param value  The value to be used for the symbol.  Whether the symbol is a terminal
 * symbol or a non-terminal symbol is determined by its value: terminal symbols have
 * "small" values (i.e. < FIRST_NONTERMINAL), and nonterminal symbols have "large" values
 * (i.e. >= FIRST_NONTERMINAL).
 * @param rule  For a terminal symbol, this parameter should be NULL.  For a nonterminal
 * symbol, this parameter can be used to specify a rule having that nonterminal at its head.
 * In that case, the reference count of the rule is increased by one and a pointer to the rule
 * is stored in the symbol.  This parameter can also be NULL for a nonterminal symbol if the
 * associated rule is not currently known and will be assigned later.
 * @return  A pointer to the new symbol, whose value and rule fields have been initialized
 * according to the parameters passed, and with other fields zeroed.  If the symbol storage
 * is exhausted and a new symbol cannot be created, then a message is printed to stderr and
 * abort() is called.
 *
 * When this function is called, if there are any recycled symbols, then one of those is removed
 * from the recycling list and used to satisfy the request.
 * Otherwise, if there currently are no recycled symbols, then a new symbol is allocated from
 * the main symbol_storage array and the num_symbols variable is incremented to record the
 * allocation.
 */
SYMBOL *new_symbol(int value, SYMBOL *rule)
{
    SYMBOL *sym;

    if (last_recycled_symbol != NULL)
    {
        sym = last_recycled_symbol;
        last_recycled_symbol = last_recycled_symbol->next;

        if (value >= FIRST_NONTERMINAL)
        {

            if (rule == NULL)
                sym->refcnt = 0;
            
            else
            {
                sym->rule = rule;
                rule->refcnt++;
                sym->refcnt = 0;
            }
        }
        else
        {
            sym->refcnt = 0;
            sym->rule = NULL;
        }
    }
    else if (num_symbols < MAX_SYMBOLS)
    {
        sym = symbol_storage + num_symbols++;

        if (value >= FIRST_NONTERMINAL)
        {
            if (rule == NULL)
                sym->refcnt = 0;
            
            else
            {
                sym->rule = rule;
                rule->refcnt++;
                sym->refcnt = 0;
            }
        }
        else
        {
            sym->refcnt = 0;
            sym->rule = NULL;
        }
    }
    else
    {
        fprintf(stderr, "The symbol storage is exhausted and a new symbol cannot be created");
        abort();
    }

    sym->value = value;
    sym->next = NULL;
    sym->nextr = NULL;
    sym->prevr = NULL;
    sym->prev = NULL;
    return sym;
}

void add_symbol(SYMBOL *sym, SYMBOL *rule)
{
    SYMBOL *last_symbol = rule->prev;

    sym->next = rule;
    sym->prev = last_symbol;
    last_symbol->next = sym;
    rule->prev = sym;
}

/**
 * Recycle a symbol that is no longer being used.
 *
 * @param s  The symbol to be recycled.  The caller must not use this symbol any more
 * once it has been recycled.
 *
 * Symbols being recycled are added to the recycled_symbols list, where they will
 * be made available for re-allocation by a subsequent call to new_symbol.
 * The recycled_symbols list is managed as a LIFO list (i.e. a stack), using the
 * next field of the SYMBOL structure to chain together the entries.
 */
void recycle_symbol(SYMBOL *s)
{
    if (last_recycled_symbol == NULL)
    {
        last_recycled_symbol = s;
        s->next = NULL;
    }
    else
    {
        s->next = last_recycled_symbol;
        last_recycled_symbol = s;
    }
}