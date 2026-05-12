/* =========================================================================
 * defs.h - Foundational type definitions shared by every translation unit.
 * =========================================================================
 *
 * This header is the lowest layer of the compiler. It introduces the three
 * core data types that every other module reads from or writes to:
 *
 *   1. Kind   - what KIND of name was declared (variable, constant, function)
 *   2. Type   - what data TYPE that name holds (bool, int, float, char,
 *               string, void) - used for type-checking in semantic actions
 *   3. Value  - a tagged-union: a Type plus the actual data bits, plus a
 *               "label" string that names this value in the emitted IR
 *               (e.g. a temp like "t3" or a variable name like "a")
 *
 * Why a separate header?  Because BOTH the symbol table (SymbolTableDefs/)
 * AND the parser actions (Parser.y) need these types, and we want to avoid
 * a circular include chain. This file pulls in only the C standard headers
 * it needs and is included by everything else.
 *
 * Convention: this file is guarded with DEFS_H to prevent double-inclusion
 * when a translation unit transitively pulls it in through multiple paths.
 * ========================================================================= */

#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>     /* FILE*, fprintf, fopen ... used everywhere      */
#include <stdlib.h>    /* malloc, free, exit, atoi, atof                  */
#include <string.h>    /* strcmp, strcpy, strdup, strlen                  */
#include <math.h>      /* pow() - used by the POW (^) operator action     */

/* -------------------------------------------------------------------------
 * Kind - which "category" of identifier a Symbol represents.
 *
 *   KIND_VAR    - a regular variable (mutable, must be declared, may or
 *                 may not be initialised at declaration time)
 *   KIND_CONST  - a "const" declaration: must have an initialiser, cannot
 *                 be assigned to after declaration
 *   KIND_FUNC   - a function declaration / definition
 *
 * Used by every grammar rule that touches an identifier so we can reject
 * nonsensical operations like "assign a value to a function name".
 * ------------------------------------------------------------------------- */
typedef enum {
    KIND_VAR,
    KIND_CONST,
    KIND_FUNC
} Kind;

/* -------------------------------------------------------------------------
 * Type - the data type of a Value.
 *
 * TYPE_VOID exists only for function return types ("void foo()"). Variables
 * cannot be declared as void; the grammar rejects that in the declaration
 * action by raising a semantic error.
 *
 * Ordering is meaningful only insofar as TYPE_INT and TYPE_FLOAT are the
 * two "numeric" types; the arithmetic-operator actions promote INT->FLOAT
 * when one of the operands is FLOAT.
 * ------------------------------------------------------------------------- */
typedef enum {
    TYPE_BOOL,     /* true / false (stored in data.i as 0 or 1)            */
    TYPE_INT,      /* signed integer (stored in data.i)                    */
    TYPE_FLOAT,    /* IEEE-754 float (stored in data.f)                    */
    TYPE_CHAR,     /* single character literal (stored in data.c)          */
    TYPE_STRING,   /* heap-allocated C string (stored in data.s)           */
    TYPE_VOID      /* only valid for function return types                 */
} Type;

/* -------------------------------------------------------------------------
 * Value - the unit of currency between grammar rules.
 *
 * Every expression in the language produces a Value*. The %union in
 * Parser.y declares (Value*) as one of the possible semantic-attribute
 * types so that any expression rule can pass one upward.
 *
 *   label  - the *name* that this value is referred to by in the emitted
 *            quadruples. For a constant operand this is its literal text
 *            ("5", "true"), for a variable it's the variable's name ("a"),
 *            and for an intermediate result of an operator it is a freshly
 *            allocated temp name like "t0", "t1", "t2"...  The label is
 *            what we print in the (op, arg1, arg2, result) quadruple.
 *
 *   type   - the Type of this value, used for type-checking before emission
 *
 *   data   - the actual bits. Tagged-union: the field you read MUST match
 *            the type. Reading data.f when type==TYPE_INT is undefined.
 *
 * Note: 'label' is a heap-allocated string most of the time. Some special
 * cases (the constant strings "tr" and "sr") leak memory by overwriting
 * the malloc'd pointer with a string literal -- a known bug, documented
 * elsewhere; left alone here so we don't break call sites.
 * ------------------------------------------------------------------------- */
typedef struct {
    char* label;       /* identifier name used in emitted quadruples       */
    Type  type;        /* what tag the union below carries                 */
    union {
        int   i;       /* TYPE_BOOL (0/1) and TYPE_INT                     */
        float f;       /* TYPE_FLOAT                                       */
        char  c;       /* TYPE_CHAR                                        */
        char* s;       /* TYPE_STRING - heap-allocated, owned by parser    */
    } data;
} Value;

#endif /* DEFS_H */
