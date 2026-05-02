# make_icon.ps1 — Generates assets\seng_icon.ico from website\logo.png
# Run from repo root: powershell -ExecutionPolicy Bypass -File assets\make_icon.ps1

param([string]$Root = (Split-Path -Parent $PSScriptRoot))

$PngPath = Join-Path $Root "website\logo.png"
$IcoPath = Join-Path $Root "assets\seng_icon.ico"

if (-not (Test-Path $PngPath)) {
    Write-Host "  ERROR: logo.png not found at $PngPath" -ForegroundColor Red
    exit 1
}

Add-Type -AssemblyName System.Drawing

try {
    $sizes = @(16, 32, 48, 64, 128, 256)
    $ms = New-Object System.IO.MemoryStream

    # ICO file format header
    $writer = New-Object System.IO.BinaryWriter($ms)
    $writer.Write([uint16]0)      # Reserved
    $writer.Write([uint16]1)      # Type: ICO
    $writer.Write([uint16]$sizes.Count)  # Image count

    # We'll collect each PNG image blob and write directory + data
    $images = @()
    foreach ($sz in $sizes) {
        $src = [System.Drawing.Image]::FromFile($PngPath)
        $bmp = New-Object System.Drawing.Bitmap($sz, $sz, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $g = [System.Drawing.Graphics]::FromImage($bmp)
        $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $g.DrawImage($src, 0, 0, $sz, $sz)
        $g.Dispose(); $src.Dispose()

        $imgMs = New-Object System.IO.MemoryStream
        $bmp.Save($imgMs, [System.Drawing.Imaging.ImageFormat]::Png)
        $bmp.Dispose()
        $images += ,$imgMs.ToArray()
        $imgMs.Dispose()
    }

    # We need the data offset = 6 (header) + 16*count (directory entries)
    $dataOffset = 6 + 16 * $sizes.Count

    # Write directory entries
    for ($i = 0; $i -lt $sizes.Count; $i++) {
        $sz = $sizes[$i]
        $data = $images[$i]
        $w = if ($sz -eq 256) { 0 } else { $sz }
        $h = if ($sz -eq 256) { 0 } else { $sz }
        $writer.Write([byte]$w)          # Width
        $writer.Write([byte]$h)          # Height
        $writer.Write([byte]0)           # Colour count
        $writer.Write([byte]0)           # Reserved
        $writer.Write([uint16]1)         # Planes
        $writer.Write([uint16]32)        # Bit count
        $writer.Write([uint32]$data.Length)  # Data size
        $writer.Write([uint32]$dataOffset)   # Data offset
        $dataOffset += $data.Length
    }

    # Write image data blobs
    foreach ($data in $images) {
        $writer.Write($data)
    }

    $writer.Flush()
    [System.IO.File]::WriteAllBytes($IcoPath, $ms.ToArray())
    $ms.Dispose(); $writer.Dispose()

    Write-Host "  Icon generated: $IcoPath ($($sizes.Count) sizes)" -ForegroundColor Green
} catch {
    Write-Host "  ERROR: $_" -ForegroundColor Red
    exit 1
}
