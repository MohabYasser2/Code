/* =========================================================================
 * SymbolTable.c - The stack-of-scopes implementation.
 *
 * The cleverness here is minimal: just a linked stack of
 * ScopeSymbolTable instances. The interesting behaviour is in
 * SymbolTable_get (which walks the chain) and SymbolTable_pop (which
 * triggers the unused-symbol warnings via Symbol_destroy).
 * ========================================================================= */

#include "SymbolTable.h"

/* Construct: one empty scope on the stack (the global scope). size=1. */
SymbolTable* SymbolTable_construct() {
    SymbolTable* symbolTable = malloc(sizeof(SymbolTable));
    symbolTable->head = ScopeSymbolTable_construct();
    symbolTable->size = 1;
    return symbolTable;
}

/* Insert into the current (innermost) scope. The caller has already
 * verified there's no collision in THIS scope; we just hand it off. */
void SymbolTable_insert(SymbolTable* symbolTable, Symbol* symbol) {
    ScopeSymbolTable* scopeSymbolTable = symbolTable->head;
    ScopeSymbolTable_insert(scopeSymbolTable, symbol);
}

/* -------------------------------------------------------------------------
 * SymbolTable_get - lexical scope lookup.
 *
 * Starts at the innermost scope (`head`) and follows the chain outward
 * through every enclosing scope until either the name is found or we
 * run off the end (global scope's next is NULL).
 *
 * This is what implements "inner declarations shadow outer ones": the
 * first hit wins because we never see the outer copy if a closer one
 * exists.
 *
 * Returns NULL if the name is undefined in every visible scope; the
 * caller turns that NULL into a "cannot find symbol" semantic error.
 * ------------------------------------------------------------------------- */
Symbol* SymbolTable_get(SymbolTable* symbolTable, char* name) {
    ScopeSymbolTable* currentScopeSymbolTable = symbolTable->head;
    while (currentScopeSymbolTable) {
        Symbol* symbol = ScopeSymbolTable_get(currentScopeSymbolTable, name);
        if (symbol) {
            return symbol;                              /* shadowing hit */
        }
        currentScopeSymbolTable = currentScopeSymbolTable->next;
    }
    return NULL;
}

/* -------------------------------------------------------------------------
 * SymbolTable_push - open a fresh inner scope on '{'.
 *
 * Allocate a new ScopeSymbolTable, link its `next` to the current head,
 * then make it the new head. size increments so log lines can indent.
 * ------------------------------------------------------------------------- */
void SymbolTable_push(SymbolTable* symbolTable) {
    ScopeSymbolTable* scopeSymbolTable = ScopeSymbolTable_construct();
    scopeSymbolTable->next = symbolTable->head;     /* chain outward     */
    symbolTable->head      = scopeSymbolTable;      /* it's now innermost */
    symbolTable->size      = symbolTable->size + 1;
}

/* -------------------------------------------------------------------------
 * SymbolTable_pop - close the innermost scope on '}'.
 *
 * Important detail: we DETACH the top before destroying it, by setting
 * its `next` to NULL. This ensures ScopeSymbolTable_destroy does NOT
 * recurse into the enclosing scope (which would obliterate the whole
 * stack). Destroying the detached scope is what triggers the
 * "unused symbol" warnings for everything declared in that block.
 * ------------------------------------------------------------------------- */
void SymbolTable_pop(SymbolTable* symbolTable) {
    ScopeSymbolTable* scopeSymbolTable = symbolTable->head;
    symbolTable->head      = scopeSymbolTable->next;  /* restore enclosing */
    scopeSymbolTable->next = NULL;                    /* break the chain   */
    ScopeSymbolTable_destroy(scopeSymbolTable);       /* runs warnings     */
    symbolTable->size      = symbolTable->size - 1;
}

/* End-of-program destructor. Recursively frees everything still on the
 * stack (typically just the global scope by the time we reach here). */
void SymbolTable_destroy(SymbolTable* symbolTable) {
    ScopeSymbolTable_destroy(symbolTable->head);
    free(symbolTable);
}
