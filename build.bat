@echo off
REM ─────────────────────────────────────────────────────────────────────────────
REM  seng Build Script for Windows
REM  Author  : NoCorps.org build by KANAGARAJ-M
REM  Website : https://nocorps.org/seng
REM ─────────────────────────────────────────────────────────────────────────────

setlocal EnableDelayedExpansion

set INSTALL_DIR=%~1
if "%INSTALL_DIR%"=="" set INSTALL_DIR=%~dp0

set SRC_DIR=%INSTALL_DIR%src
set OUT_EXE=%INSTALL_DIR%seng.exe

echo.
echo  Building seng from source...
echo  Source : %SRC_DIR%
echo  Output : %OUT_EXE%
echo.

REM ── Try GCC first ────────────────────────────────────────────────────────────
where gcc >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo  [GCC] Found gcc — compiling...
    gcc -std=c99 -O2 -I"%SRC_DIR%" ^
        "%SRC_DIR%\common.c" ^
        "%SRC_DIR%\lexer.c"  ^
        "%SRC_DIR%\ast.c"    ^
        "%SRC_DIR%\parser.c" ^
        "%SRC_DIR%\value.c"  ^
        "%SRC_DIR%\env.c"    ^
        "%SRC_DIR%\interp.c" ^
        "%SRC_DIR%\compiler.c" ^
        "%SRC_DIR%\vm.c"     ^
        "%SRC_DIR%\main.c"   ^
        -o "%OUT_EXE%" -lm
    if !ERRORLEVEL! EQU 0 (
        echo  [GCC] Build successful: %OUT_EXE%
        goto :success
    ) else (
        echo  [GCC] Build failed. Trying Clang...
    )
)

REM ── Try Clang ────────────────────────────────────────────────────────────────
where clang >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo  [Clang] Found clang — compiling...
    clang -std=c99 -O2 -I"%SRC_DIR%" ^
        "%SRC_DIR%\common.c" ^
        "%SRC_DIR%\lexer.c"  ^
        "%SRC_DIR%\ast.c"    ^
        "%SRC_DIR%\parser.c" ^
        "%SRC_DIR%\value.c"  ^
        "%SRC_DIR%\env.c"    ^
        "%SRC_DIR%\interp.c" ^
        "%SRC_DIR%\compiler.c" ^
        "%SRC_DIR%\vm.c"     ^
        "%SRC_DIR%\main.c"   ^
        -o "%OUT_EXE%" -lm
    if !ERRORLEVEL! EQU 0 (
        echo  [Clang] Build successful: %OUT_EXE%
        goto :success
    ) else (
        echo  [Clang] Build failed.
    )
)

REM ── Try MSVC (cl.exe) ────────────────────────────────────────────────────────
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo  [MSVC] Found cl.exe — compiling...
    cl /nologo /O2 /I"%SRC_DIR%" ^
        "%SRC_DIR%\common.c" "%SRC_DIR%\lexer.c" "%SRC_DIR%\ast.c" ^
        "%SRC_DIR%\parser.c" "%SRC_DIR%\value.c" "%SRC_DIR%\env.c" ^
        "%SRC_DIR%\interp.c" "%SRC_DIR%\compiler.c" "%SRC_DIR%\vm.c" "%SRC_DIR%\main.c" ^
        /Fe:"%OUT_EXE%"
    if !ERRORLEVEL! EQU 0 (
        echo  [MSVC] Build successful: %OUT_EXE%
        goto :success
    )
)

echo.
echo  ERROR: No C compiler (gcc / clang / cl) was found on PATH.
echo  Please install GCC (https://winlibs.com) or Clang and re-run.
echo  Or download the pre-built seng.exe from: https://nocorps.org/seng
echo.
exit /b 1

:success
echo.
echo  seng built successfully!
echo  Run:  seng help
echo.
exit /b 0
