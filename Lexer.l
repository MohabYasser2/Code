/* =========================================================================
 * Lexer.l - has regex(lang keywords/consts/operators) that will be captured and send as tokens to parser.
 * =========================================================================
 *
 *  file has three sections separated by "%%":
 *
 *   1. DEFINITIONS  - C code where anything inside it is copied directly into generated c file lex.yy.c,
 *                     Also optional named regex aliases. We use this section only to pull in
 *                     the token names defined by Bison (Parser.tab.h).
 *
 *   2. RULES        - REGEX  { C action }
 *                     Each time flex finds the longest prefix match for
 *                     one of these regexes, it runs the C action.
 *                     - yytext is a char* with the matched text
 *                     - yylval is common part with Bison; we save
 *                       the parsed value here so the parser can use it
 *                     - return TOKEN_NAME hands the token to yyparse()
 *
 *   3. SUBROUTINES  - extra C functions (we provide yywrap).
 *
 * RULE ORDER MATTERS: flex picks the LONGEST match; on ties, it picks the FIRST rule listed.
 * ========================================================================= */

/* ---- Section 1: Definitions ------------------------------------------- */
%{
    #include "utils.h"

    /* This brings in the token Definitions (BOOL, INTEGER, IF, ...) generated ,without it lexer wont know what (int , if,...)means
     * by `bison -d Parser.y`--> so we can `return` them by name below. */
    #include "Parser.tab.h"
%}

/* ---- Section 2: Rules ------------------------------------------------- */
%%

    /* ----- Comments: silently consumed, no token returned ------------- */
"//".*                                              { /* single-line comment */ }
"/*"([^*]|(\*)[^/])*"*/"                            { /* multi-line  comment */ }

    /* ----- Literals --------------------------------------------------- */

    /* Boolean literal. We stash 1 for "true", 0 for "false" into
     * yylval.i, because BOOL is declared with <i> in the %union. */
("false"|"true")                                    {yylval.i = (yytext[0] == 't'); return BOOL;}

    /* Integer literal: atoi() turns the matched digits into an int. */
[0-9]+                                              {yylval.i = atoi(yytext); return INTEGER;}

    /* Float literal: must have a dot AND digits on both sides. atof()
     * does the C-string-to-double conversion. */
[0-9]+\.[0-9]+                                      {yylval.f = atof(yytext); return FLOAT;}

     /* Character literal examples:
     *   'a'   -> normal char
     *   '\n'  -> escaped char
     * We decode the escape sequence manually because flex doesn't.
     * The matched text always has the surrounding quotes, so yytext[1]
     * is the first char inside; if that char is '\\' the escape code
     * is yytext[2]. */
\'([^\']|\\.)\'                                     {
                                                        char result;
                                                        if (yytext[1] == '\\') {
                                                            /* Backslash escape - decode the second char. */
                                                            switch (yytext[2]) {
                                                                case 'n':  result = '\n'; break; /*new line*/
                                                                case 't':  result = '\t'; break; /*tab escape*/
                                                                case 'r':  result = '\r'; break; /*carriage return*/
                                                                case '0':  result = '\0'; break; /*null char*/
                                                                case '\\': result = '\\'; break; /*backslash*/
                                                                case '\'': result = '\''; break; /*single qoute*/
                                                                case '\"': result = '\"'; break; /*double quote*/
                                                                default:
                                                                    /* Unknown escape: hard fail to avoid silently mis-tokenising. */
                                                                    fprintf(stderr, "Unknown escape sequence: \\%c\n", yytext[2]);
                                                                    exit(1);
                                                            }
                                                        }
                                                        else {
                                                            /* Plain character literal. */
                                                            result = yytext[1];
                                                        }
                                                        yylval.c = result;
                                                        return CHAR;
                                                    }

    /* String literal: a double quote, any number of non-quote chars or
     * backslash-escapes, then a closing quote.
     * Process:
     *   (a) strdup the matched text starting at yytext+1 (skipping the
     *       opening "), then null out the last char to drop the closing ".
     *   (b) walk the result, copying each char into a fresh buffer,
     *       translating any backslash-escape we encounter.
     *   (c) store the pointer in yylval.s.
    */
\"([^\"]|\\.)*\"                                    {
                                                        char *content = strdup(yytext + 1); /*move ptr to drop start "*/
                                                        content[strlen(content) - 1] = '\0'; /* replace with string terminator \0 to drop trailing " */
                                                        char *decoded = (char *)malloc(strlen(content) + 1); /*create new mem for final string i/p 2 chars->after decode 1 char*/
                                                        int decoded_index = 0;
                                                        for (int i = 0; i < strlen(content); i++) {
                                                            if (content[i] == '\\' && i + 1 < strlen(content)) { /*if curr is backslash and next char exists*/
                                                                /* Two-char escape - decode it. */
                                                                switch (content[i + 1]) {
                                                                    case 'n':  decoded[decoded_index++] = '\n'; i++; break; /*Adds one REAL newline character to decoded string and skip next char*/
                                                                    case 't':  decoded[decoded_index++] = '\t'; i++; break;
                                                                    case 'r':  decoded[decoded_index++] = '\r'; i++; break;
                                                                    case '0':  decoded[decoded_index++] = '\0'; i++; break;
                                                                    case '\\': decoded[decoded_index++] = '\\'; i++; break;
                                                                    case '\"': decoded[decoded_index++] = '\"'; i++; break;
                                                                    case '\'': decoded[decoded_index++] = '\''; i++; break;
                                                                    default:
                                                                        fprintf(stderr, "Unknown escape sequence: \\%c\n", content[i + 1]);
                                                                        exit(1);
                                                                }
                                                            }
                                                            else {
                                                                /* Plain character. */
                                                                decoded[decoded_index++] = content[i];
                                                            }
                                                        }
                                                        decoded[decoded_index] = '\0'; /*adds null termnator*/
                                                        yylval.s = decoded;
                                                        return STRING;
                                                    }

    /* ----- Type names --------------------------------------------------
     * Type keywords (int, float, bool, char, string).
     * All are returned as TYPE token.
     * yylval.s stores the actual type name using strdup(),
     * since yytext is reused by flex after each match(everytime lexer read new token).
     * so for example token:TYPE and value: "int"
    */
("bool"|"int"|"float"|"char"|"string")              {yylval.s = strdup(yytext); return TYPE;}

    /* ----- Reserved keywords ------------------------------------------ */
"void"                                              {return VOID;}
"const"                                             {return CONST;}
"if"                                                {return IF;}
"else"                                              {return ELSE;}
"switch"                                            {return SWITCH;}
"case"                                              {return CASE;}
"default"                                           {return DEFAULT;}
"for"                                               {return FOR;}
"from"                                              {return FROM;}
"to"                                                {return TO;}
"step"                                              {return STEP;}
"while"                                             {return WHILE;}
"repeat"                                            {return REPEAT;}
"until"                                             {return UNTIL;}
"return"                                            {return RETURN;}
"print"                                             {return PRINT;}

    /* ----- Punctuation ------------------------------------------------ */
";"                                                 {return SEMICOLON;}
","                                                 {return COMMA;}
"("                                                 {return OPENING_PARENTHESIS;}
")"                                                 {return CLOSING_PARENTHESIS;}
"{"                                                 {return SCOPE_START;}
"}"                                                 {return SCOPE_END;}

    /* ----- Single-character arithmetic operators ---------------------- */
"+"                                                 {return PLUS;}
"-"                                                 {return MINUS;}
"*"                                                 {return MULT;}
"/"                                                 {return DIV;}
"%"                                                 {return MOD;}
"^"                                                 {return POW;}

    /* ----- Logical operators (single character) ----------------------- */
"&"                                                 {return AND;}
"|"                                                 {return OR;}
"!"                                                 {return NOT;}

    /* ----- Two-char comparisons MUST come before single-char ones ----
     * Flex prefers the longest match, but if we put "<" first and "<="
     * second, a tie wouldn't happen (longest still wins); however listing
     * the multi-char operators near each other is good defensive style. */
"<="                                                {return LE;}
">="                                                {return GE;}
"=="                                                {return EQ;}
"!="                                                {return NE;}
"<"                                                 {return LT;}
">"                                                 {return GT;}
"="                                                 {return ASSIGN;}

    /* ----- Identifier --------------------------------------------------
     * Must come AFTER all keywords/type-names above, otherwise flex would
     * happily match "int" as an IDENTIFIER (still a tie, but listed-first
     * matters here). 
     * strdup() copies the matched name so it survives beyond the next yylex() call (which would change yytext).
    */
[a-zA-Z_][a-zA-Z0-9_]*                              {yylval.s = strdup(yytext); return IDENTIFIER;}

    /* ----- Whitespace --------------------------------------------------
     * Spaces and tabs are silently dropped (no token returned). Newlines
     * bump the global `line` counter so error messages have accurate
     * line numbers. */
[ \t]+                                              { /* ignore whitespace */ }
\n                                                  {line++;}
%%

/* ---- Section 3: Subroutines ------------------------------------------ */

/* yywrap is called when flex hits end of input. Returning
 * 1 tells flex "no more input streams
  */
int yywrap(void) {
    return 1;
}
