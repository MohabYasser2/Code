/* =========================================================================
 * Symbol.c - Implementations for Symbol construction and destruction.
 * ========================================================================= */

#include "Symbol.h"
#include "../globals.h"

/* -------------------------------------------------------------------------
 * Symbol_construct
 *
 * Allocates a fresh Symbol on the heap, copies every field, and returns
 * the pointer. The grammar action is responsible for keeping the
 * `name` string alive (it's already heap-allocated by the lexer via
 * strdup), so we just store the pointer.
 *
 * For function symbols (numParams > 0) we duplicate the parameter
 * pointer-array via memcpy so the symbol owns its own copy. The Symbol*
 * elements themselves are *shared* (we do not deep-copy individual
 * parameter Symbols).
 * ------------------------------------------------------------------------- */
Symbol* Symbol_construct(char* name, Kind kind, int isInit, int declLine,
                         Value value, Symbol** params, int numParams) {
    Symbol* symbol   = malloc(sizeof(Symbol));
    symbol->name     = name;
    symbol->kind     = kind;
    symbol->isInit   = isInit;
    symbol->declLine = declLine;
    symbol->isUsed   = 0;                /* nothing has read it yet         */
    symbol->value    = value;

    /* Copy the parameter array if this is a function with parameters. */
    if (numParams > 0) {
        symbol->params = malloc(numParams * sizeof(Symbol*));
        memcpy(symbol->params, params, numParams * sizeof(Symbol*));
    }
    symbol->numParams = numParams;
    symbol->next      = NULL;            /* not in any hash bucket yet      */
    return symbol;
}

/* -------------------------------------------------------------------------
 * Symbol_destroy
 *
 * Frees the symbol and (recursively) everything it owns. Called by the
 * scope's destructor when a block closes.
 *
 * Side effect: if `verbose` is set and the symbol was never read, write
 * a warning to the semantic-analysis log. THIS is how the compiler
 * detects "you declared `x` but never used it" - we don't track usage
 * with a separate pass; instead the warning is naturally emitted at
 * the moment the scope dies, exactly when we know nothing else will
 * ever reference this symbol.
 *
 * Recursion:
 *   - For each parameter (function symbols only) we destroy with
 *     verbose=0; parameters live as long as the function symbol and we
 *     don't want spurious "parameter unused" warnings.
 *   - The `next` chain is the hash-bucket linked list, so destroying
 *     the head of a bucket destroys the entire bucket.
 * ------------------------------------------------------------------------- */
void Symbol_destroy(Symbol* symbol, int verbose) {
    if (verbose && !symbol->isUsed) {
        fprintf(semanticAnalysisFile,
                "Warning: Unused symbol \"%s\" at line %d\n",
                symbol->name, symbol->declLine);
    }

    /* Parameters of a function (silently destroyed). */
    for (int i = 0; i < symbol->numParams; i++) {
        Symbol_destroy(symbol->params[i], 0);
    }
    if (symbol->numParams > 0) {
        free(symbol->params);
    }

    /* Next symbol in this hash bucket (with warnings). */
    if (symbol->next) {
        Symbol_destroy(symbol->next, 1);
    }

    free(symbol);
}
