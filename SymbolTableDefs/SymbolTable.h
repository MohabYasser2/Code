/* =========================================================================
 * SymbolTable.h - The compiler-wide symbol table: a stack of scopes.
 * =========================================================================
 *
 * This is the public face of "the symbol table". Internally it's a stack
 * of ScopeSymbolTable instances, where:
 *
 *   - The TOP scope is the innermost block we're currently parsing
 *     (e.g. the body of the current if-branch).
 *   - Each scope's `next` pointer points to its enclosing scope.
 *   - The BOTTOM of the stack is the global scope, set up by main().
 *
 * Operations:
 *
 *   construct  - build a stack containing one empty global scope
 *   insert     - add a Symbol to the TOP (current) scope
 *   get        - look up a name, walking from the top scope outward to
 *                the global scope (lexical scoping)
 *   push       - on every '{' grammar action; opens a fresh inner scope
 *   pop        - on every '}' grammar action; destroys the top scope
 *                and emits "unused" warnings for any unread symbols
 *   destroy    - tear down all remaining scopes; called at program end
 *
 * `size` tracks how deep we are. It's incremented on push, decremented
 * on pop, and used by the verbose output to indent log lines.
 * ========================================================================= */

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "ScopeSymbolTable.h"

typedef struct {
    ScopeSymbolTable* head;     /* top of the stack = innermost scope */
    int size;                   /* number of scopes currently open    */
} SymbolTable;

/* Build a fresh table with one (empty) global scope. size = 1. */
SymbolTable* SymbolTable_construct();

/* Insert a Symbol into the CURRENT top scope. Caller is responsible
 * for having already checked that the name isn't already there
 * (using ScopeSymbolTable_get on table->head). */
void SymbolTable_insert(SymbolTable* symbolTable, Symbol* symbol);

/* Look up `name` by walking scopes from innermost to outermost.
 * Returns the FIRST hit (lexical scoping = inner shadows outer).
 * Returns NULL if not found anywhere visible. */
Symbol* SymbolTable_get(SymbolTable* symbolTable, char* name);

/* Open a new inner scope. Called by the `block` grammar rule when '{'
 * is consumed. The new scope becomes the new head; the old head
 * remains reachable as new->next. */
void SymbolTable_push(SymbolTable* symbolTable);

/* Close the innermost scope. Called when '}' is consumed. Detaches the
 * top, restores its enclosing scope as the new head, then destroys
 * the detached scope - which is what triggers unused-symbol warnings
 * for everything that lived inside that block. */
void SymbolTable_pop(SymbolTable* symbolTable);

/* Tear down the entire stack. Called at the end of main(). */
void SymbolTable_destroy(SymbolTable* symbolTable);

#endif /* SYMBOLTABLE_H */
