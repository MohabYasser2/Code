/* =========================================================================
 * globals.h - Shared mutable state accessible from every translation unit.
 * =========================================================================
 *
 * Bison generates a single C function (yyparse) that runs all of our
 * semantic actions. Because those actions are scattered across hundreds
 * of grammar rules and need to cooperate (e.g. one rule reserves a label,
 * another emits the label definition later), we keep cross-action state in
 * a small set of GLOBAL variables declared here and DEFINED in globals.c.
 *
 * Every variable below is `extern` so each .c file gets the declaration
 * but only globals.c provides storage - that's the standard C pattern for
 * sharing one variable across many translation units.
 *
 * The cost of globals is high coupling. The benefit is that any grammar
 * action can read/mutate the parser's compile-time state with no parameter
 * threading. Acceptable for a teaching compiler; in a production design we
 * would wrap these in a "Compiler" context struct and pass it everywhere.
 * ========================================================================= */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "SymbolTableDefs/SymbolTable.h"

/* -------------------------------------------------------------------------
 * Output streams - opened in main(), closed there (and in yyerror on early
 * abort). Each one collects one kind of compiler artefact:
 *
 *   symbolTableFile        - human-readable log of declarations, indented
 *                            by scope depth ("SymbolTable.out")
 *   semanticAnalysisFile   - one line per semantic error or warning
 *                            ("SemanticAnalysis.out")
 *   symbolTableVisualiser  - tabular view of every scope's contents
 *                            ("SymbolTableVisualiser.out")
 *   quadruplesFile         - the actual IR output: one quadruple per line
 *                            ("Quadruples.out")
 * ------------------------------------------------------------------------- */
extern FILE* symbolTableFile;
extern FILE* semanticAnalysisFile;
extern FILE* symbolTableVisualiser;
extern FILE* quadruplesFile;

/* -------------------------------------------------------------------------
 * Source-position tracking. The lexer bumps `line` on every '\n' so that
 * error messages can say "Line 17: ...".
 * ------------------------------------------------------------------------- */
extern int line;

/* -------------------------------------------------------------------------
 * The compiler's THE symbol table - a stack of per-scope hash tables.
 * Pushed when a '{' opens a block, popped when '}' closes it.
 * ------------------------------------------------------------------------- */
extern SymbolTable* symbolTable;

/* -------------------------------------------------------------------------
 * Parameter / argument counters. The grammar accumulates these while
 * reducing parameter_list / argument_list. Used by:
 *   - function_definition to know how many params were collected
 *   - function_call       to verify argument count matches parameter count
 *
 * Reset to 0 implicitly when the next list rule starts collecting again.
 * ------------------------------------------------------------------------- */
extern int numParams;
extern int numArgs;

/* -------------------------------------------------------------------------
 * lastSymbol - the most recently inserted Symbol*. Used by the `block`
 * rule to detect "we just declared a function and are now entering its
 * body": when the next '{' opens a scope we need to inject the function's
 * parameters as locals into that new scope.
 *
 * IMPORTANT lifecycle: lastSymbol is set by every declaration action and
 * is CLEARED inside `block` once the FUNC-body case has consumed it.
 * This narrow lifetime ("set by the last declaration, valid only until
 * the next `{`") is what prevents the nested-function check from
 * spuriously firing on every inner if / while / for / switch / repeat
 * block opened inside a function body.
 * ------------------------------------------------------------------------- */
extern Symbol* lastSymbol;

/* -------------------------------------------------------------------------
 * Function-context tracking:
 *   currFunc  - non-NULL while we are inside a function body; the
 *               return_statement rule consults it to type-check the
 *               returned expression and to reject `return` at top level
 *   funcDepth - nesting depth of {} inside a function body. Allows us
 *               to distinguish "the function's outermost block closing"
 *               (which should reset currFunc) from "an inner block".
 * ------------------------------------------------------------------------- */
extern Symbol* currFunc;
extern int     funcDepth;

/* -------------------------------------------------------------------------
 * Temp-variable counter. allocateTempVar() reads & bumps this each time
 * an expression reduction needs a fresh "tN" name in the IR. Reset to 0
 * after each top-level statement so each statement starts at t0.
 * ------------------------------------------------------------------------- */
extern int tempVars;

/* -------------------------------------------------------------------------
 * Label generator + stack. We emit labels named "LABEL0", "LABEL1", ...
 *
 *   labels      - global counter, monotonically increasing
 *   labelNames  - stack of pending label names that have been *reserved*
 *                 but not yet *defined*. Used by control-flow rules
 *                 (if/while/for/switch/repeat) to remember where the
 *                 "else" / "end" target will eventually land. This is
 *                 standard backpatching with a small bounded stack.
 *   labelDepth  - top-of-stack index for labelNames
 *
 * The fixed size of 1000 caps total reserved labels in flight at once;
 * far beyond what any real source program will need.
 * ------------------------------------------------------------------------- */
extern int   labels;
extern char* labelNames[1000];
extern int   labelDepth;

/* -------------------------------------------------------------------------
 * Switch-statement state. To support NESTED switches, we save the prior
 * "current switch discriminant" each time we enter a new switch.
 *
 *   switchExpr  - per-nesting-level: the Value* that the user's switch()
 *                 was discriminating on (we copy it into special var "sr")
 *   switchLabel - per-nesting-level: the labelDepth index that holds the
 *                 "end of this switch" label
 *   switchDepth - top-of-stack index for both arrays
 *   numCases    - per-nesting-level: how many `case` arms this switch has
 * ------------------------------------------------------------------------- */
extern Value* switchExpr[100];
extern int    switchLabel[100];
extern int    switchDepth;
extern int    numCases[100];

/* -------------------------------------------------------------------------
 * For-loop state. To support NESTED for-loops, we save the iterator name
 * for each level (used in the step-increment quadruple at loop-end).
 * ------------------------------------------------------------------------- */
extern char* forIterator[100];
extern int   forDepth;

#endif /* GLOBALS_H */
