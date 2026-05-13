# C-Like Language Compiler

This project is a **compiler for a C-like programming language** built using **Lex (Flex)** for lexical analysis and **Yacc (Bison)** for parsing. It translates high-level code into an intermediate representation, and lays the groundwork for further stages such as code optimization.

## 🔧 Features

- Lexical analysis using **Lex/Flex**
- Syntax parsing using **Yacc/Bison**
- Support for:
  - Variable declarations
  - Arithmetic expressions
  - Control structures (`if`, `else`, `while`)
  - Function declarations and calls
  - Simple I/O (`print`)
- Semantic analysis:
  - Conflicting declarations
  - Type and scope checking
  - Uninitialized and unused variables
- Intermediate representation (IR) generation
- Error handling with line number reporting

## 📁 Project Structure

```
.
├── Makefile          # Build automation
├── Lexer.l           # Lex file (token definitions)
├── Parser.y          # Yacc file (grammar rules)
├── defs.h            # Definitions and imports
├── globals.h         # Global variables declaration
├── globals.c         # Global variables definition
├── utils.h           # Helper functions declaration
├── utils.c           # Helper functions definition
├── SymbolTableDefs/  # Definitions for symbol table
│ ├── Symbol.h
│ ├── Symbol.c
│ ├── SymbolList.h
│ ├── SymbolList.c
│ ├── ScopeSymbolTable.h
│ ├── ScopeSymbolTable.c
│ ├── SymbolTable.h
│ └── SymbolTable.c
└── README.md         # Project documentation
````

## 🚀 Getting Started

### Prerequisites

- `flex` (Lex implementation)
- `bison` (Yacc implementation)
- `gcc`
- `make` (optional, for easier builds)

### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/MarwannAhmed/compiler.git
   cd compiler
   ```

2. Run the project using `make`:

   ```bash
   make
   ```

   This will:

   * Run `bison` on `Parser.y`
   * Run `flex` on `Lexer.l`
   * Compile all `.c` files
   * Run the final executable named `Compiler.exe` on a source code file named `src.txt`

## 📝 Example Input

```c
int a = 5;
int b = 10;
if (a < b) {
    print(a + b);
}
```

## ✅ Output in SymbolTableVisualiser.out
```
Symbol table for global scope.
| name         | kind      | value        | type   | declaration line  |
------------------------------------------------------------------------
| a            | variable  | 5            | int    | 1                 |
| b            | variable  | 10           | int    | 2                 |
    Symbol table for new scope: 1
    | name         | kind      | value        | type   | declaration line  |
    ------------------------------------------------------------------------
Returning to symbol table for global scope.
```

## ⚠️ Error Handling

The compiler reports lexical, syntax and semantic errors with line numbers and messages.



## 🔤 Token Descriptions

The following tokens are recognized by the lexical analyzer (defined in `Lexer.l`) using **Flex**:

### 🔹 Literals

| Token     | Description                          | Example          |
| --------- | ------------------------------------ | ---------------- |
| `INTEGER` | Integer literal                      | `42`             |
| `FLOAT`   | Floating-point literal               | `3.14`           |
| `CHAR`    | Character literal (supports escapes) | `'a'`, `'\n'`    |
| `STRING`  | String literal (supports escapes)    | `"Hello\nWorld"` |
| `BOOL`    | Boolean literal                      | `true`, `false`  |

### 🔹 Keywords

| Token     | Description                                               |
| --------- | --------------------------------------------------------- |
| `TYPE`    | Type specifiers: `int`, `bool`, `float`, `char`, `string` |
| `VOID`    | `void` keyword                                            |
| `CONST`   | `const` keyword                                           |
| `IF`      | `if` conditional                                          |
| `ELSE`    | `else` block                                              |
| `SWITCH`  | `switch` statement                                        |
| `CASE`    | `case` in switch                                          |
| `DEFAULT` | `default` in switch                                       |
| `FOR`     | `for` loop                                                |
| `FROM`    | `from` (for loop start)                                   |
| `TO`      | `to` (for loop end)                                       |
| `STEP`    | `step` (for loop step)                                    |
| `WHILE`   | `while` loop                                              |
| `REPEAT`  | `repeat` loop start                                       |
| `UNTIL`   | `until` loop condition                                    |
| `RETURN`  | `return` statement                                        |
| `PRINT`   | `print` output                                            |

### 🔹 Operators and Symbols

| Token                  | Symbol      | Description               |
| ---------------------- | ----------- | ------------------------- |
| `ASSIGN`               | `=`         | Assignment                |             
| `PLUS`, `MINUS`        | `+`, `-`    | Arithmetic                |             
| `MULT`, `DIV`, `MOD`   | `* / %`     | Multiplication & Division |            
| `POW`                  | `^`         | Power (exponentiation)    |             
| `LT`, `LE`, `GT`, `GE` | `< <= > >=` | Comparison                |             
| `EQ`, `NE`             | `== !=`     | Equality                  |             
| `AND`, `OR`, `NOT`     | `& \| !`    | Logical ops               |
| `SEMICOLON`            | `;`         | Statement terminator      |             
| `COMMA`                | `,`         | Separator                 |             
| `OPENING_PARENTHESIS`  | `(`         | Grouping or call          |             
| `CLOSING_PARENTHESIS`  | `)`         | Grouping or call          |             
| `SCOPE_START`          | `{`         | Block start               |             
| `SCOPE_END`            | `}`         | Block end                 |             

### 🔹 Identifiers and Whitespace

| Token        | Description                                         |
| ------------ | --------------------------------------------------- |
| `IDENTIFIER` | User-defined names (variables, functions, constants)|
| (whitespace) | Spaces, tabs, newlines — ignored                    |

### 🔹 Comments

* **Single-line comments** start with `//` and are ignored.
* **Multi-line comments** use `/* ... */` and are ignored.

## 🧮 Intermediate Representation: Quadruples

The compiler translates high-level code into **quadruples**, an intermediate representation (IR) that simplifies expressions and control flow into atomic instructions. These are useful for further stages such as optimization or target code generation.

### 🔹 Quadruple Format

Each quadruple is structured as:
```
(operation, argument1, argument2, result)
```

- `operation`: The action performed (e.g. `=`, `+`, `JMP`, `CALL`, etc.)
- `argument1`: The first operand (or source)
- `argument2`: The second operand (if applicable)
- `result`: The destination where the result is stored or the control jumps

When a field is not required (e.g. unary operations or jumps), it is denoted as `N/A`.

### 🔹 Quadruple Reference Table

| Quadruple                      | Description                                       |
|------------------------------- |---------------------------------------------------|
| (=, value, N/A, variable)      | Assigns `value` to `variable`                     |
| (+, x, y, result)              | Arithmetic addition: `x + y`                      |
| (-, x, y, result)              | Arithmetic subtraction: `x - y`                   |
| (*, x, y, result)              | Arithmetic multiplication: `x * y`                |
| (/, x, y, result)              | Arithmetic division: `x / y`                      |
| (%, x, y, result)              | Arithmetic remainder: `x % y` (for integers only) |
| (^, x, y, result)              | Arithmetic exponentiation: `x ^ y`                |
| (&, x, y, result)              | Logical AND of `x` and `y`                        |
| (\|, x, y, result)             | Logical OR of `x` and `y`                         |
| (!, x, N/A, result)            | Logical NOT of `x`                                |
| (-, x, N/A, result)            | Unary negation of `x`                             |
| (==, x, y, result)             | Comparison `x == y`                               |
| (!=, x, y, result)             | Comparison `x != y`                               |
| (<, x, y, result)              | Comparison `x < y`                                |
| (>, x, y, result)              | Comparison `x > y`                                |
| (<=, x, y, result)             | Comparison `x <= y`                               |
| (>=, x, y, result)             | Comparison `x >= y`                               |
| (JMP, label, N/A, N/A)         | Unconditional jump to label                       |
| (JZ, label, N/A, N/A)          | Jump to label if last result is zero (false)      |
| (PUSH, x, N/A, N/A)            | Pushes `x` onto call stack                        |
| (POP, N/A, N/A, variable)      | Pops top of stack into `variable`                 |
| (CALL, func, N/A, N/A)         | Calls function `func`                             |
| (RET, N/A, N/A, N/A)           | Returns from function                             |
| (PRINT, x, N/A, N/A)           | Prints `x`                                        |

### 🔹 Special Variables and Labels

* **Branching targets** are denoted as labels written in all caps and followed by a colon (example: `LABEL0:`).
* **Function definitions** start with `PROC` followed by function name and end with `RET`.
* **Function return values** are stored in special variable `tr`.
* **Switch expressions** are stored in special variable `sr`.

to run the gui command : python playground_server.py