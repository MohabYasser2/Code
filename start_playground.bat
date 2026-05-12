@echo off
REM ==========================================================================
REM Compiler Playground launcher.
REM
REM Usage:  double-click, or run in a terminal.
REM
REM Builds Compiler.exe if missing, then starts the Python server.
REM The browser opens automatically to http://localhost:5050.
REM ==========================================================================

cd /d "%~dp0"

REM ---- ensure Compiler.exe exists; if not, try to build it -----------------
if not exist Compiler.exe (
    echo Compiler.exe not found. Attempting to build...

    REM Add GnuWin32 to PATH if it's there (using 8.3 short path to avoid
    REM the m4 / spaces-in-path bug). Adjust if your bison/flex live elsewhere.
    set "PATH=C:\PROGRA~2\GnuWin32\bin;C:\msys64\ucrt64\bin;%PATH%"
    set "BISON_PKGDATADIR=C:\PROGRA~2\GnuWin32\share\bison"

    bison.exe -d -v Parser.y
    if errorlevel 1 ( echo bison failed & pause & exit /b 1 )
    flex.exe Lexer.l
    if errorlevel 1 ( echo flex failed & pause & exit /b 1 )
    gcc.exe SymbolTableDefs/Symbol.c SymbolTableDefs/SymbolList.c SymbolTableDefs/ScopeSymbolTable.c SymbolTableDefs/SymbolTable.c globals.c utils.c Parser.tab.c lex.yy.c -o Compiler.exe
    if errorlevel 1 ( echo gcc failed & pause & exit /b 1 )
    echo Build OK.
)

REM ---- start the Python server ---------------------------------------------
where py >nul 2>nul
if %errorlevel%==0 (
    py playground_server.py
) else (
    python playground_server.py
)
