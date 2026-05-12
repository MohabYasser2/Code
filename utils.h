/* =========================================================================
 * utils.h - Small helper functions used by grammar actions in Parser.y.
 * =========================================================================
 *
 * Keeping these out of Parser.y means the grammar file stays focused on
 * grammar + semantic actions, and the helpers can be unit-tested or
 * reviewed independently.
 *
 *   allocateTempVar(n*)         - mint a fresh "tN" name for an IR temp
 *   writeSymbolToVisualiser()   - pretty-print one Symbol row to the
 *                                 SymbolTableVisualiser.out file
 *
 * We include globals.h because writeSymbolToVisualiser writes to a FILE*
 * declared there.
 * ========================================================================= */

#ifndef UTILS_H
#define UTILS_H

#include "globals.h"

/* -------------------------------------------------------------------------
 * allocateTempVar - return a freshly heap-allocated string "tN" and bump *n.
 *
 * Used by every operator action that produces an intermediate result.
 * The counter `n` is the global `tempVars` (reset to 0 after each
 * top-level statement) so each statement's IR starts at t0.
 *
 * Caller owns the returned pointer; the parser typically stores it as the
 * `label` field of a freshly-malloc'd Value.
 * ------------------------------------------------------------------------- */
char* allocateTempVar(int* n);

/* -------------------------------------------------------------------------
 * writeSymbolToVisualiser - emit one tabular row describing `symbol` to
 * the SymbolTableVisualiser.out file, indented to `depth` levels.
 *
 * Columns: name | kind | value | type | declaration line
 * Renders the union according to symbol->value.type; prints "-" if the
 * symbol has never been initialised or is of type VOID (functions).
 * ------------------------------------------------------------------------- */
void writeSymbolToVisualiser(Symbol* symbol, int depth);

#endif /* UTILS_H */
