
#Requires -Version 5.1
<#
.SYNOPSIS
    seng (Simple English Programming Language) — Windows Installer
    Author : NoCorps.org build by KANAGARAJ-M
    Version: 1.0.0
    GitHub : https://github.com/KANAGARAJ-M/SENG
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ── colours ──────────────────────────────────────────────────────────────────
function Write-Cyan  ($msg) { Write-Host $msg -ForegroundColor Cyan }
function Write-Green ($msg) { Write-Host $msg -ForegroundColor Green }
function Write-Yellow($msg) { Write-Host $msg -ForegroundColor Yellow }
function Write-Red   ($msg) { Write-Host $msg -ForegroundColor Red }
function Write-White ($msg) { Write-Host $msg -ForegroundColor White }

# ── banner ────────────────────────────────────────────────────────────────────
Clear-Host
Write-Cyan  "  ╔══════════════════════════════════════════════════════╗"
Write-Cyan  "  ║                                                      ║"
Write-Cyan  "  ║    ████████╗███████╗███╗   ██╗ ██████╗              ║"
Write-Cyan  "  ║    ██╔════╝██╔════╝████╗  ██║██╔════╝              ║"
Write-Cyan  "  ║    ███████╗█████╗  ██╔██╗ ██║██║  ███╗             ║"
Write-Cyan  "  ║    ╚════██║██╔══╝  ██║╚██╗██║██║   ██║             ║"
Write-Cyan  "  ║    ███████║███████╗██║ ╚████║╚██████╔╝             ║"
Write-Cyan  "  ║    ╚══════╝╚══════╝╚═╝  ╚═══╝ ╚═════╝              ║"
Write-Cyan  "  ║                                                      ║"
Write-Cyan  "  ║      Simple English Programming Language             ║"
Write-Cyan  "  ║      Version 1.0.0  ·  by NoCorps.org build by KANAGARAJ-M                ║"
Write-Cyan  "  ║      MIT License  ·  github.com/KANAGARAJ-M/SENG        ║"
Write-Cyan  "  ║                                                      ║"
Write-Cyan  "  ╚══════════════════════════════════════════════════════╝"
Write-Host ""

# ── locate script root ────────────────────────────────────────────────────────
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ScriptDir) { $ScriptDir = Get-Location }

# ── licence display ───────────────────────────────────────────────────────────
$LicensePath = Join-Path $ScriptDir "LICENSE"

Write-Cyan "  ┌─ LICENSE ────────────────────────────────────────────────┐"
if (Test-Path $LicensePath) {
    Get-Content $LicensePath | ForEach-Object { Write-White "  │  $_" }
} else {
    Write-White "  │  MIT License — Copyright (c) 2026 NoCorps.org build by KANAGARAJ-M"
    Write-White "  │  Permission is granted, free of charge, to use,"
    Write-White "  │  copy, modify and distribute this software."
    Write-White "  │  THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY."
}
Write-Cyan "  └──────────────────────────────────────────────────────────┘"
Write-Host ""

# ── licence acceptance ────────────────────────────────────────────────────────
Write-Yellow "  Do you accept the MIT License? (yes/no): " -NoNewline
Write-Host ""
$accept = Read-Host "  > "
if ($accept -notmatch '^(yes|y)$') {
    Write-Red "  Installation cancelled. You must accept the license to install seng."
    exit 1
}
Write-Green "  ✓ License accepted."
Write-Host ""

# ── check for gcc ─────────────────────────────────────────────────────────────
Write-Cyan "  [1/4] Checking for GCC compiler..."
try {
    $gccVer = & gcc --version 2>&1 | Select-Object -First 1
    Write-Green "  ✓ Found: $gccVer"
} catch {
    Write-Red "  ✗ GCC not found!"
    Write-Yellow "  Please install GCC (MinGW / MSYS2) and re-run."
    Write-Yellow "  Download: https://winlibs.com  or  choco install mingw"
    exit 1
}

# ── set install directory ─────────────────────────────────────────────────────
$InstallDir = "$env:ProgramFiles\seng"
Write-Cyan "  [2/4] Install directory: $InstallDir"

if (-not (Test-Path $InstallDir)) {
    try {
        New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
        Write-Green "  ✓ Created $InstallDir"
    } catch {
        Write-Red "  ✗ Cannot create '$InstallDir'. Re-run as Administrator."
        exit 1
    }
}

# ── compile seng ──────────────────────────────────────────────────────────────
Write-Cyan "  [3/4] Compiling seng from source..."

$SrcDir = Join-Path $ScriptDir "src"
if (-not (Test-Path $SrcDir)) {
    Write-Red "  ✗ Source directory 'src\' not found next to the installer."
    Write-Yellow "  Please run the installer from the seng project root directory."
    exit 1
}

$SrcFiles = @(
    "common.c","lexer.c","ast.c","parser.c",
    "value.c","env.c","interp.c","compiler.c","vm.c","main.c"
) | ForEach-Object { Join-Path $SrcDir $_ }

$OutExe = Join-Path $InstallDir "seng.exe"

$gccArgs = @("-std=c99", "-O2", "-Isrc") + $SrcFiles + @("-o", $OutExe, "-lm")

try {
    Push-Location $ScriptDir
    $proc = Start-Process -FilePath "gcc" -ArgumentList $gccArgs `
            -Wait -PassThru -NoNewWindow `
            -RedirectStandardError "$env:TEMP\seng_build_err.txt"
    Pop-Location

    if ($proc.ExitCode -ne 0) {
        $errMsg = Get-Content "$env:TEMP\seng_build_err.txt" -Raw
        Write-Red "  ✗ Compilation failed:"
        Write-Red $errMsg
        exit 1
    }
    Write-Green "  ✓ Compiled successfully → $OutExe"
} catch {
    Pop-Location
    Write-Red "  ✗ Build error: $_"
    exit 1
}

# ── copy examples ─────────────────────────────────────────────────────────────
$ExamplesTarget = Join-Path $InstallDir "examples"
$ExamplesSrc    = Join-Path $ScriptDir "examples"
if (Test-Path $ExamplesSrc) {
    Copy-Item -Path $ExamplesSrc -Destination $ExamplesTarget -Recurse -Force
    Write-Green "  ✓ Copied examples → $ExamplesTarget"
}

# Copy LICENSE and README
foreach ($f in @("LICENSE","README.md")) {
    $fp = Join-Path $ScriptDir $f
    if (Test-Path $fp) { Copy-Item $fp $InstallDir -Force }
}

# ── add to PATH ───────────────────────────────────────────────────────────────
Write-Cyan "  [4/4] Adding seng to system PATH..."

$SysPath = [System.Environment]::GetEnvironmentVariable("Path","Machine")

if ($SysPath -split ";" | Where-Object { $_ -eq $InstallDir }) {
    Write-Green "  ✓ Already in PATH."
} else {
    try {
        $NewPath = ($SysPath.TrimEnd(";") + ";" + $InstallDir)
        [System.Environment]::SetEnvironmentVariable("Path", $NewPath, "Machine")
        $env:Path += ";$InstallDir"
        Write-Green "  ✓ Added to system PATH."
    } catch {
        Write-Yellow "  ⚠ Could not update system PATH (need Admin). Adding to user PATH instead..."
        $UserPath = [System.Environment]::GetEnvironmentVariable("Path","User")
        if (-not ($UserPath -split ";" | Where-Object { $_ -eq $InstallDir })) {
            [System.Environment]::SetEnvironmentVariable("Path", ($UserPath + ";" + $InstallDir), "User")
        }
        $env:Path += ";$InstallDir"
        Write-Green "  ✓ Added to user PATH."
    }
}

# ── create Start Menu shortcut ────────────────────────────────────────────────
try {
    $StartMenu = "$env:ProgramData\Microsoft\Windows\Start Menu\Programs\seng"
    if (-not (Test-Path $StartMenu)) { New-Item -ItemType Directory $StartMenu -Force | Out-Null }
    $WScriptShell = New-Object -ComObject WScript.Shell
    $Shortcut = $WScriptShell.CreateShortcut("$StartMenu\seng.lnk")
    $Shortcut.TargetPath       = $OutExe
    $Shortcut.WorkingDirectory = $InstallDir
    $Shortcut.Description      = "seng - Simple English Programming Language"
    $Shortcut.Save()
    Write-Green "  ✓ Start Menu shortcut created."
} catch {
    Write-Yellow "  ⚠ Could not create Start Menu shortcut (non-critical)."
}

# ── verify installation ───────────────────────────────────────────────────────
Write-Host ""
Write-Cyan "  ╔══════════════════════════════════════════════════════╗"
Write-Green "  ║           seng installed successfully! 🎉            ║"
Write-Cyan  "  ╚══════════════════════════════════════════════════════╝"
Write-Host ""
Write-White "  Installation path : $InstallDir"
Write-White "  Executable        : $OutExe"
Write-Host ""
Write-Cyan  "  Quick Start:"
Write-White "  1. Open a NEW terminal (so PATH updates take effect)"
Write-White "  2. Create a file  →  hello.se"
Write-White "     Contents:"
Write-White '         say "Hello, World!"'
Write-White "  3. Run it         →  seng hello.se"
Write-White "  4. Compile it     →  seng compile hello.se"
Write-White "  5. Run bytecode   →  seng run hello.sec"
Write-Host ""
Write-Cyan  "  Examples: $ExamplesTarget"
Write-Cyan  "  Docs    : https://github.com/KANAGARAJ-M/SENG"
Write-Host ""
Write-Yellow "  Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
