/* =========================================================================
 * SymbolList.c - Implementation of a single hash-bucket as a linked list.
 *
 * All four routines below are textbook singly-linked-list operations.
 * The interesting bit is the destructor: it delegates to Symbol_destroy
 * with verbose=1 so the "unused symbol" warning is emitted exactly when
 * the enclosing scope dies.
 * ========================================================================= */

#include "SymbolList.h"

/* Allocate an empty bucket. */
SymbolList* SymbolList_construct() {
    SymbolList* symbolList = malloc(sizeof(SymbolList));
    symbolList->head = NULL;
    symbolList->size = 0;
    return symbolList;
}

/* Insert at head. Newest declaration shadows older ones in the bucket -
 * but the caller (ScopeSymbolTable_insert) is expected to have already
 * checked for same-name duplicates and rejected them as a semantic
 * error, so we never actually rely on the head-is-newest behaviour. */
void SymbolList_insert(SymbolList* symbolList, Symbol* symbol) {
    symbol->next     = symbolList->head;
    symbolList->head = symbol;
    symbolList->size = symbolList->size + 1;
}

/* Linear walk - O(bucket size) - looking for a symbol whose name
 * matches the requested key. The hash table keeps buckets small
 * (sub-table size = 100), so in practice each bucket has 0-3 entries. */
Symbol* SymbolList_get(SymbolList* symbolList, char* name) {
    Symbol* currentSymbol = symbolList->head;
    while (currentSymbol) {
        if (strcmp(currentSymbol->name, name) == 0) {
            return currentSymbol;
        }
        currentSymbol = currentSymbol->next;
    }
    return NULL;
}

/* Recursive destroy: Symbol_destroy itself walks `next` to free the
 * whole bucket chain, so we just hand it the head and free the list
 * header. The "verbose=1" inside Symbol_destroy is what triggers the
 * "Warning: Unused symbol" log entries for symbols that were never
 * read during the scope's lifetime. */
void SymbolList_destroy(SymbolList* symbolList) {
    if (symbolList->head) {
        Symbol_destroy(symbolList->head, 1);
    }
    free(symbolList);
}
