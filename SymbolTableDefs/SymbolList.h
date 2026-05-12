/* =========================================================================
 * SymbolList.h - Linked list of Symbols used as ONE hash-table bucket.
 * =========================================================================
 *
 * ScopeSymbolTable.h has a fixed-size hash table of 100 buckets; each
 * bucket is a SymbolList. When two distinct names hash to the same
 * bucket index (a collision) they end up in the same SymbolList chain
 * and are distinguished by strcmp on the `name` field.
 *
 * The implementation is a classic singly-linked list with a head pointer
 * and a size counter (size is informational; the destructor doesn't
 * need it).
 * ========================================================================= */

#ifndef SYMBOLLIST_H
#define SYMBOLLIST_H

#include "Symbol.h"

typedef struct {
    Symbol* head;       /* first symbol in the bucket (NULL == empty)  */
    int     size;       /* number of symbols in the bucket             */
} SymbolList;

/* Construct an empty bucket. */
SymbolList* SymbolList_construct();

/* Insert at head (O(1)).  Caller already verified that there's no name
 * collision via SymbolList_get - the caller is the scope-level rule
 * that has already rejected duplicates within the scope. */
void SymbolList_insert(SymbolList* symbolList, Symbol* symbol);

/* Linear search by name (O(bucket size)). Returns NULL if not found. */
Symbol* SymbolList_get(SymbolList* symbolList, char* name);

/* Recursively destroy every symbol and then free the list header. */
void SymbolList_destroy(SymbolList* symbolList);

#endif /* SYMBOLLIST_H */
