/* =========================================================================
 * ScopeSymbolTable.h - One scope's symbol table: a 100-bucket hash table.
 * =========================================================================
 *
 * Each scope (function body, if-block, loop body, manually-introduced
 * { ... } block) has its own ScopeSymbolTable. The full compiler-wide
 * symbol table (SymbolTable.h) is a STACK of these, with the innermost
 * scope on top.
 *
 * Implementation:
 *   - Hash table of 100 SymbolList buckets, indexed by djb2(name) % 100.
 *   - Buckets handle collisions via linked-list chaining.
 *   - The `next` pointer hooks one ScopeSymbolTable to the next-deeper
 *     scope on the stack; lookup in SymbolTable_get walks this chain
 *     outward until it finds the name or exhausts all visible scopes.
 *
 * Why 100 buckets?  Small constant - tiny memory footprint, plenty of
 * spread for any realistic source program. No resizing logic needed.
 * ========================================================================= */

#ifndef SCOPESYMBOLTABLE_H
#define SCOPESYMBOLTABLE_H

#include "SymbolList.h"

typedef struct ScopeSymbolTable {
    SymbolList* table[100];                   /* the 100 hash buckets       */
    struct ScopeSymbolTable* next;            /* enclosing scope (or NULL)  */
} ScopeSymbolTable;

/* Allocate an empty scope: 100 empty buckets, next pointer = NULL. */
ScopeSymbolTable* ScopeSymbolTable_construct();

/* djb2 hash (Daniel J. Bernstein's classic).
 * Returns an integer in [0, 99] so it can index the bucket array. */
unsigned int ScopeSymbolTable_hash(char* name);

/* Insert into the appropriate bucket. No duplicate-name check - that
 * is done one layer up (in the grammar action that calls this) with
 * ScopeSymbolTable_get returning non-NULL = redeclaration error. */
void ScopeSymbolTable_insert(ScopeSymbolTable* scopeSymbolTable,
                             Symbol* symbol);

/* Look up a name in THIS scope only (does not chain to enclosing scopes).
 * Used by redeclaration checks ("is `x` already declared in this exact
 * scope?"). Cross-scope lookup is done by SymbolTable_get. */
Symbol* ScopeSymbolTable_get(ScopeSymbolTable* scopeSymbolTable,
                             char* name);

/* Destroy this scope (every bucket + their symbols), then recursively
 * destroy any chained scope. In practice we only destroy the top scope
 * at pop-time, so `next` is NULL and recursion only kicks in at program
 * end when we tear down the whole stack at once. */
void ScopeSymbolTable_destroy(ScopeSymbolTable* scopeSymbolTable);

#endif /* SCOPESYMBOLTABLE_H */
