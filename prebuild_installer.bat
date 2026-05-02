@echo off
REM ─────────────────────────────────────────────────────────────────────────────
REM  seng — Pre-build script for Inno Setup
REM  Run this BEFORE opening seng_installer.iss in Inno Setup Compiler
REM  Author  : NoCorps.org build by KANAGARAJ-M
REM  Website : https://nocorps.org/seng
REM ─────────────────────────────────────────────────────────────────────────────
echo.
echo  ════════════════════════════════════════════════════
echo   seng — Pre-build for Inno Setup Installer
echo   Copyright (C) 2026 NoCorps.org build by KANAGARAJ-M (nocorps.org)
echo  ════════════════════════════════════════════════════
echo.

REM ── 1. Create output directories ─────────────────────────────────────────────
if not exist installer mkdir installer
if not exist assets    mkdir assets
echo  [1/3] Output directories ready.

REM ── 2. Compile seng.exe ───────────────────────────────────────────────────────
echo  [2/3] Compiling seng.exe...
call build.bat .
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo  ERROR: Build failed. Install GCC or Clang and try again.
    echo  Download GCC: https://winlibs.com
    pause
    exit /b 1
)
echo  [2/3] seng.exe compiled successfully.

REM ── 3. Generate icon from logo ────────────────────────────────────────────────
echo  [3/3] Generating icon...
powershell -ExecutionPolicy Bypass -NoProfile -File "%~dp0assets\make_icon.ps1" -Root "%~dp0"
if %ERRORLEVEL% NEQ 0 (
    echo  WARNING: Icon generation failed - installer will compile without custom icon.
)

echo.
echo  ════════════════════════════════════════════════════
echo   Pre-build complete! Now open Inno Setup:
echo   1. Launch Inno Setup Compiler
echo   2. File ^> Open ^> seng_installer.iss
echo   3. Build ^> Compile  (or press F9)
echo   4. Installer → installer\seng-setup-1.0.0-windows-x64.exe
echo  ════════════════════════════════════════════════════
echo.
pause
