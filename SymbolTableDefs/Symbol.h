/* =========================================================================
 * Symbol.h - The Symbol struct: one entry per declared name.
 * =========================================================================
 *
 * "Symbol" is our jargon for a single declaration the compiler has seen.
 * Every variable, every constant, every function becomes one Symbol; that
 * Symbol gets inserted into the current scope and looked up later by name.
 *
 * Why this layout?
 *   - `name`       lookup key (must be unique within one scope)
 *   - `kind`       distinguishes variable / constant / function so the
 *                  assignment rule can reject "func = 3" or "const = 5"
 *   - `isInit`     1 once the variable has been assigned a value; the
 *                  primary rule rejects reading an uninitialised name
 *   - `declLine`   source line where it was declared, used in error msgs
 *                  and in the SymbolTableVisualiser output
 *   - `isUsed`     set to 1 whenever someone READS this symbol (it gets
 *                  used in an expression, passed as an argument, etc.).
 *                  When the scope dies we walk all its symbols and emit
 *                  a "Warning: Unused symbol" for any with isUsed == 0
 *   - `value`      the type tag + the actual constant-folded value (the
 *                  parser tracks values at compile time for literal
 *                  expressions, useful for divide-by-zero detection)
 *   - `params`     for functions only: an array of Symbol* describing
 *                  each parameter (name + declared type)
 *   - `numParams`  size of the params array
 *   - `next`       hash-bucket chaining pointer; lets SymbolList store
 *                  multiple Symbols that hash to the same bucket
 *
 * This struct is the *only* place that holds compiler-time knowledge
 * about a name. Look it up in the symbol table to learn anything about
 * an identifier mentioned in the source.
 * ========================================================================= */

#ifndef SYMBOL_H
#define SYMBOL_H

#include "../defs.h"

typedef struct Symbol {
    char*           name;        /* identifier as it appears in source     */
    Kind            kind;        /* VAR / CONST / FUNC                     */
    int             isInit;      /* 1 once value has been assigned         */
    int             declLine;    /* source line of the declaration         */
    int             isUsed;      /* 1 once name has been read somewhere    */
    Value           value;       /* type tag + current value (constant-fold)*/
    struct Symbol** params;      /* function parameters (FUNC only)        */
    int             numParams;   /* number of entries in params            */
    struct Symbol*  next;        /* hash-bucket chaining (SymbolList)      */
} Symbol;

/* -------------------------------------------------------------------------
 * Symbol_construct - heap-allocate and initialise a new Symbol.
 *
 * For variables / constants pass params=NULL, numParams=0.
 * For functions pass an array of parameter-Symbols you've already built
 * up in the parameter_list grammar reduction. The function does a
 * memcpy of that array into its own freshly-allocated buffer, so the
 * caller can free / reuse theirs.
 * ------------------------------------------------------------------------- */
Symbol* Symbol_construct(char* name, Kind kind, int isInit, int declLine,
                         Value value, Symbol** params, int numParams);

/* -------------------------------------------------------------------------
 * Symbol_destroy - recursively free a Symbol and everything it owns.
 *
 *   verbose - 1 means "emit a warning if this symbol is isUsed==0"; 0
 *             means "stay quiet" (we use 0 for parameter Symbols stored
 *             inside another Symbol so we don't double-warn).
 *
 * Walks `params` (for function symbols) and then `next` (the hash-bucket
 * chain) so destroying the head of a bucket destroys the whole bucket.
 * ------------------------------------------------------------------------- */
void Symbol_destroy(Symbol* symbol, int verbose);

#endif /* SYMBOL_H */
