
#Requires -Version 5.1
<#
.SYNOPSIS
  seng Uninstaller — removes seng from your system
  Author: NoCorps.org build by KANAGARAJ-M
#>

$InstallDir = "$env:ProgramFiles\seng"

Write-Host ""
Write-Host "  seng Uninstaller" -ForegroundColor Cyan
Write-Host "  ─────────────────────────────────" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path $InstallDir)) {
    Write-Host "  seng does not appear to be installed." -ForegroundColor Yellow
    exit 0
}

Write-Host "  Remove seng from $InstallDir? (yes/no): " -ForegroundColor Yellow -NoNewline
$confirm = Read-Host
if ($confirm -notmatch '^(yes|y)$') {
    Write-Host "  Uninstall cancelled." -ForegroundColor Yellow
    exit 0
}

# Remove from PATH
foreach ($scope in @("Machine","User")) {
    $p = [System.Environment]::GetEnvironmentVariable("Path",$scope)
    $newP = ($p -split ";" | Where-Object { $_ -ne $InstallDir }) -join ";"
    [System.Environment]::SetEnvironmentVariable("Path",$newP,$scope)
}
Write-Host "  ✓ Removed from PATH" -ForegroundColor Green

# Remove files
Remove-Item -Path $InstallDir -Recurse -Force
Write-Host "  ✓ Removed $InstallDir" -ForegroundColor Green

# Remove Start Menu
$sm = "$env:ProgramData\Microsoft\Windows\Start Menu\Programs\seng"
if (Test-Path $sm) { Remove-Item $sm -Recurse -Force; Write-Host "  ✓ Removed Start Menu entry" -ForegroundColor Green }

Write-Host ""
Write-Host "  seng has been uninstalled. Goodbye! 👋" -ForegroundColor Cyan
Write-Host ""
