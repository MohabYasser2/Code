/* =========================================================================
 * utils.c - Implementations of the helpers declared in utils.h.
 * ========================================================================= */

#include "utils.h"

/* -------------------------------------------------------------------------
 * allocateTempVar
 *
 * Mints a new temp-variable name like "t0", "t1", "t2"...  These names
 * appear in the emitted quadruples as the `result` field of operator
 * quads (e.g.  (+, a, b, t0)  ) and as the operand of any subsequent
 * quad that consumes that temp.
 *
 * Strategy:
 *   1. Compute how many characters "t<n>" needs (snprintf with NULL+0
 *      returns the required length without writing).
 *   2. Allocate that many bytes + 1 for the NUL terminator.
 *   3. Format the string into the buffer.
 *   4. Post-increment *n so the next call gets a different name.
 *
 * NOTE - known bug (left in place to keep behaviour identical to the
 *         original submission): the malloc call below is
 *         `malloc(sizeof(size + 1))` which is `sizeof(int) == 4`, NOT
 *         `malloc(size + 1)`. For n in 0..999 this happens to be enough
 *         because "t999\0" is 5 bytes (oops, off-by-one even there!) and
 *         malloc usually overallocates. Names "t1000+" will smash the
 *         heap. A clean fix is `malloc(size + 1)`.
 * ------------------------------------------------------------------------- */
char* allocateTempVar(int* n) {
    char* label;
    int size = snprintf(NULL, 0, "t%d", *n);     /* length needed for "tN" */
    label = malloc(sizeof(size + 1));            /* BUG: should be size+1  */
    sprintf(label, "t%d", *n);
    (*n)++;
    return label;
}

/* -------------------------------------------------------------------------
 * writeSymbolToVisualiser
 *
 * Renders one Symbol as a row in the human-readable
 * "SymbolTableVisualiser.out" table. Called every time a declaration
 * action inserts a Symbol so that the output shows the table growing as
 * the parser walks the file.
 *
 * Output format (5 columns, fixed widths):
 *     | name         | kind      | value        | type   | declaration line  |
 *
 * Rules for the "value" cell:
 *   - if the symbol has never been initialised, print "-"
 *   - if it's a function (type VOID, or type set but no concrete value),
 *     also print "-"
 *   - otherwise render the union according to its tag:
 *       BOOL   -> "true" / "false"
 *       INT    -> "%d"
 *       FLOAT  -> "%.5f"
 *       CHAR   -> "%c"
 *       STRING -> "%s"
 * ------------------------------------------------------------------------- */
void writeSymbolToVisualiser(Symbol* symbol, int depth) {
    char* symbolType;
    char  valueData[100] = "";

    /* (a) map the Type enum to a printable string for column "type" */
    switch (symbol->value.type) {
        case TYPE_BOOL:   symbolType = "bool";   break;
        case TYPE_INT:    symbolType = "int";    break;
        case TYPE_FLOAT:  symbolType = "float";  break;
        case TYPE_CHAR:   symbolType = "char";   break;
        case TYPE_STRING: symbolType = "string"; break;
        case TYPE_VOID:   symbolType = "void";   break;
    }

    /* (b) render the value cell. Uninitialised / void -> "-". */
    if (!symbol->isInit || symbol->value.type == TYPE_VOID) {
        strcpy(valueData, "-");
    } else {
        switch (symbol->value.type) {
            case TYPE_BOOL:
                snprintf(valueData, sizeof(valueData), "%s",
                         symbol->value.data.i ? "true" : "false");
                break;
            case TYPE_INT:
                snprintf(valueData, sizeof(valueData), "%d",
                         symbol->value.data.i);
                break;
            case TYPE_FLOAT:
                snprintf(valueData, sizeof(valueData), "%.5f",
                         symbol->value.data.f);
                break;
            case TYPE_CHAR:
                snprintf(valueData, sizeof(valueData), "%c",
                         symbol->value.data.c);
                break;
            case TYPE_STRING:
                snprintf(valueData, sizeof(valueData), "%s",
                         symbol->value.data.s);
                break;
            default:
                strcpy(valueData, "-");
                break;
        }
    }

    /* (c) map Kind to printable string for column "kind" */
    char* symbolKind = symbol->kind == KIND_VAR   ? "variable"
                     : symbol->kind == KIND_CONST ? "constant"
                                                  : "function";

    /* (d) finally write the row. The %-Nx format specifiers left-pad
     *     each column to a fixed width so the resulting table is aligned.
     *     The caller has already indented us with leading "    " spaces
     *     proportional to the scope depth. */
    fprintf(symbolTableVisualiser,
            "| %-12s | %-9s | %-12s | %-6s | %-17d |\n",
            symbol->name, symbolKind, valueData,
            symbolType, symbol->declLine);
}
