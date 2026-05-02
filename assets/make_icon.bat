@echo off
REM ─────────────────────────────────────────────────────────────────────────────
REM  Generate seng_icon.ico from website\logo.png
REM  Run from repo root:  assets\make_icon.bat
REM ─────────────────────────────────────────────────────────────────────────────
echo.
echo  Generating seng_icon.ico...
powershell -ExecutionPolicy Bypass -NoProfile -File "%~dp0make_icon.ps1" -Root "%~dp0.."
if %ERRORLEVEL% EQU 0 (
    echo  Done. Icon saved to assets\seng_icon.ico
) else (
    echo  Icon generation failed.
    exit /b 1
)
