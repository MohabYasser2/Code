/* =========================================================================
 * globals.c - Storage definitions for the extern variables declared in
 *             globals.h.
 * =========================================================================
 *
 * One-line-per-variable file. The C language requires that each extern
 * variable has exactly one *definition* (storage allocation) in some
 * translation unit; the other .c files only see the `extern` declarations
 * in globals.h and refer to the storage that lives here.
 *
 * Initial values: NULL/0 - the parser sets them as it runs. We could
 * have let C zero-initialise them implicitly, but spelling out the zero
 * is clearer.
 * ========================================================================= */

#include "globals.h"

/* ---- output streams (filled in by main(), see Parser.y) ---------------- */
FILE* symbolTableFile        = NULL;
FILE* semanticAnalysisFile   = NULL;
FILE* symbolTableVisualiser  = NULL;
FILE* quadruplesFile         = NULL;

/* ---- source-position tracking (bumped by Lexer.l on every '\n') -------- */
int line = 1;

/* ---- THE compiler-wide symbol table (constructed in main()) ------------ */
SymbolTable* symbolTable = NULL;

/* ---- list-accumulation counters ---------------------------------------- */
int numParams = 0;     /* used while reducing parameter_list  */
int numArgs   = 0;     /* used while reducing argument_list   */

/* ---- most-recently-declared symbol (used by `block` to detect that we
 *      are entering a function body and need to inject its parameters)   */
Symbol* lastSymbol = NULL;

/* ---- per-switch state (parallel arrays indexed by switchDepth) --------- */
int    numCases[100];                /* cases collected at each nesting    */
Value* switchExpr[100];              /* the discriminant Value* per level  */
int    switchLabel[100];             /* end-of-switch label index per lvl  */
int    switchDepth = 0;              /* current nesting depth              */

/* ---- function-context tracking ----------------------------------------- */
Symbol* currFunc  = NULL;            /* NULL = not inside any function     */
int     funcDepth = 0;               /* nested { } depth inside that func  */

/* ---- IR generator state ------------------------------------------------ */
int     tempVars = 0;                /* next temp variable index (t0, t1)  */
int     labels   = 0;                /* next label number   (LABEL0,1...)  */
char*   labelNames[1000];            /* stack of pending forward-jump tgts */
int     labelDepth = 0;              /* top-of-stack index for labelNames  */

/* ---- per-for-loop state (parallel arrays indexed by forDepth) ---------- */
char*   forIterator[100];            /* iterator variable name per level   */
int     forDepth;                    /* current for-loop nesting depth     */

/* ---- global error flag ------------------------------------------------- */
int hadError = 0;                    /* set to 1 on first error            */
