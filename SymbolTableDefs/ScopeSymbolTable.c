/* =========================================================================
 * ScopeSymbolTable.c - Hash table for ONE scope.
 *
 * Each scope owns 100 SymbolList buckets. We hash a symbol's name with
 * djb2 to pick a bucket, then either insert into or search that bucket's
 * linked list. Collisions degrade to a linear walk inside the bucket -
 * fine because buckets stay tiny in practice.
 * ========================================================================= */

#include "ScopeSymbolTable.h"

/* Allocate the scope: 100 buckets, no chained-next-scope yet. */
ScopeSymbolTable* ScopeSymbolTable_construct() {
    ScopeSymbolTable* scopeSymbolTable = malloc(sizeof(ScopeSymbolTable));
    for (int i = 0; i < 100; i++) {
        scopeSymbolTable->table[i] = SymbolList_construct();
    }
    scopeSymbolTable->next = NULL;
    return scopeSymbolTable;
}

/* -------------------------------------------------------------------------
 * djb2 hash by Daniel J. Bernstein
 *
 *   hash(0) = 5381
 *   hash(i) = hash(i-1) * 33 + name[i]
 *
 * The multiplication by 33 is implemented as a left-shift-by-5 plus add
 * (`(hash << 5) + hash`) - the compiler will likely turn this back into
 * an `imul 33`, but the shift+add form is the canonical djb2 spelling.
 *
 * Returns an integer in [0, 99] so we can use it directly as a bucket
 * index. % 100 because we have 100 buckets per scope.
 * ------------------------------------------------------------------------- */
unsigned int ScopeSymbolTable_hash(char* name) {
    unsigned int hash = 5381;
    for (int i = 0; i < strlen(name); i++) {
        hash = ((hash << 5) + hash) + name[i];   /* hash * 33 + char */
    }
    return hash % 100;
}

/* Insert into the bucket selected by the name's hash. */
void ScopeSymbolTable_insert(ScopeSymbolTable* scopeSymbolTable,
                             Symbol* symbol) {
    int index = ScopeSymbolTable_hash(symbol->name);
    SymbolList_insert(scopeSymbolTable->table[index], symbol);
}

/* Look up in the bucket selected by the name's hash. Searches only THIS
 * scope - to look across enclosing scopes use SymbolTable_get instead. */
Symbol* ScopeSymbolTable_get(ScopeSymbolTable* scopeSymbolTable,
                             char* name) {
    int index = ScopeSymbolTable_hash(name);
    return SymbolList_get(scopeSymbolTable->table[index], name);
}

/* Destroy every bucket, then (recursively, if any) the chained enclosing
 * scope. The recursion only ever matters at program shutdown - the
 * SymbolTable_pop path always has next==NULL because we detach before
 * destroying. */
void ScopeSymbolTable_destroy(ScopeSymbolTable* scopeSymbolTable) {
    for (int i = 0; i < 100; i++) {
        SymbolList_destroy(scopeSymbolTable->table[i]);
    }
    if (scopeSymbolTable->next) {
        ScopeSymbolTable_destroy(scopeSymbolTable->next);
    }
    free(scopeSymbolTable);
}
