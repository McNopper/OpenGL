param(
    [int[]]$ExampleNumbers,
    [switch]$All
)

# Screenshot capture script for OpenGL examples with DPI awareness
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

Add-Type @"
    using System;
    using System.Runtime.InteropServices;
    using System.Drawing;
    
    public class Win32 {
        [DllImport("user32.dll")]
        public static extern bool SetProcessDPIAware();
        
        [DllImport("user32.dll")]
        public static extern IntPtr FindWindow(string lpClassName, string lpWindowName);
        
        [DllImport("user32.dll")]
        public static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);
        
        [DllImport("user32.dll")]
        public static extern bool ClientToScreen(IntPtr hWnd, ref POINT lpPoint);
        
        [DllImport("user32.dll")]
        public static extern IntPtr GetDC(IntPtr hWnd);
        
        [DllImport("user32.dll")]
        public static extern int ReleaseDC(IntPtr hWnd, IntPtr hDC);
        
        [DllImport("gdi32.dll")]
        public static extern bool BitBlt(IntPtr hdcDest, int xDest, int yDest, int wDest, int hDest, 
                                         IntPtr hdcSource, int xSrc, int ySrc, int RasterOp);
        
        [DllImport("gdi32.dll")]
        public static extern IntPtr CreateCompatibleDC(IntPtr hdc);
        
        [DllImport("gdi32.dll")]
        public static extern IntPtr CreateCompatibleBitmap(IntPtr hdc, int nWidth, int nHeight);
        
        [DllImport("gdi32.dll")]
        public static extern IntPtr SelectObject(IntPtr hdc, IntPtr hgdiobj);
        
        [DllImport("gdi32.dll")]
        public static extern bool DeleteObject(IntPtr hObject);
        
        [DllImport("gdi32.dll")]
        public static extern bool DeleteDC(IntPtr hdc);
        
        public const int SRCCOPY = 0x00CC0020;
    }
    
    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }
    
    public struct POINT {
        public int X;
        public int Y;
    }
"@

# Set DPI awareness to avoid scaling issues
[Win32]::SetProcessDPIAware() | Out-Null

function Capture-Window {
    param(
        [string]$ProcessName,
        [string]$OutputPath
    )
    
    Start-Sleep -Milliseconds 2000  # Wait for window to render
    
    $processes = Get-Process | Where-Object { $_.ProcessName -like "*$ProcessName*" -and $_.MainWindowHandle -ne 0 }
    
    if ($processes) {
        $process = $processes[0]
        $handle = $process.MainWindowHandle
        
        # Get client area dimensions (without window decorations)
        $rect = New-Object RECT
        [Win32]::GetClientRect($handle, [ref]$rect) | Out-Null
        
        $width = $rect.Right - $rect.Left
        $height = $rect.Bottom - $rect.Top
        
        if ($width -gt 0 -and $height -gt 0) {
            # Get DC from window
            $hdcSrc = [Win32]::GetDC($handle)
            
            # Create compatible DC and bitmap
            $hdcDest = [Win32]::CreateCompatibleDC($hdcSrc)
            $hBitmap = [Win32]::CreateCompatibleBitmap($hdcSrc, $width, $height)
            $hOld = [Win32]::SelectObject($hdcDest, $hBitmap)
            
            # Copy window content
            [Win32]::BitBlt($hdcDest, 0, 0, $width, $height, $hdcSrc, 0, 0, [Win32]::SRCCOPY) | Out-Null
            
            # Convert to .NET bitmap
            [Win32]::SelectObject($hdcDest, $hOld) | Out-Null
            $bitmap = [System.Drawing.Image]::FromHbitmap($hBitmap)
            
            # Save
            $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
            
            # Cleanup
            $bitmap.Dispose()
            [Win32]::DeleteObject($hBitmap) | Out-Null
            [Win32]::DeleteDC($hdcDest) | Out-Null
            [Win32]::ReleaseDC($handle, $hdcSrc) | Out-Null
            
            return $true
        }
    }
    
    return $false
}

# Main script
$binariesPath = Join-Path $PSScriptRoot "Binaries"
$imagesPath   = Join-Path $PSScriptRoot "screenshots"
$examples = Get-ChildItem "$binariesPath\Example*.exe" | Sort-Object Name

if (-not $examples -or $examples.Count -eq 0)
{
    Write-Error "No example executables found in $binariesPath"
    exit 1
}

$selectedExamples = $examples
$selectionDescription = "all examples"
$modeLabel = "All Examples"

if ($ExampleNumbers -and $ExampleNumbers.Count -gt 0)
{
    $uniqueExampleNumbers = $ExampleNumbers | Sort-Object -Unique
    $formattedExampleNames = $uniqueExampleNumbers | ForEach-Object { "Example{0:D2}" -f $_ }
    $selectedExamples = $examples | Where-Object { $formattedExampleNames -contains $_.BaseName }

    $missingExamples = $formattedExampleNames | Where-Object { $_ -notin $selectedExamples.BaseName }
    if ($missingExamples)
    {
        Write-Warning ("Skipping missing executables: {0}" -f ($missingExamples -join ", "))
    }

    if (-not $selectedExamples -or $selectedExamples.Count -eq 0)
    {
        Write-Error ("No matching examples found for numbers: {0}" -f ($uniqueExampleNumbers -join ", "))
        exit 1
    }

    $selectionDescription = "examples: {0}" -f ($formattedExampleNames -join ", ")
    $modeLabel = "Selected Examples"
}
elseif (-not $All)
{
    $All = $true
}

Write-Host ("`n=== Capturing Screenshots for {0} ===" -f $modeLabel) -ForegroundColor Green
Write-Host ("Target set: {0}" -f $selectionDescription) -ForegroundColor Cyan
Write-Host "DPI Aware: Enabled" -ForegroundColor Cyan
Write-Host ("Total examples: {0}`n" -f $selectedExamples.Count) -ForegroundColor Yellow

$successCount = 0
$failCount = 0

for ($position = 0; $position -lt $selectedExamples.Count; $position++)
{
    $exe = $selectedExamples[$position]
    $exeName = $exe.BaseName
    $index = [int]($exeName -replace 'Example', '')
    $current = $position + 1
    
    Write-Host ("[{0}/{1}] Capturing {2} (Example {3})..." -f $current, $selectedExamples.Count, $exeName, $index) -ForegroundColor Cyan
    
    # Start the application
    $process = Start-Process -FilePath $exe.FullName -WorkingDirectory $binariesPath -PassThru
    
    # Wait for the window to appear and render content
    Start-Sleep -Seconds 3
    
    # Capture screenshot
    $outputFile = Join-Path $imagesPath "$exeName.png"
    $captured = Capture-Window -ProcessName $exeName -OutputPath $outputFile
    
    if ($captured -and (Test-Path $outputFile)) {
        $fileSize = (Get-Item $outputFile).Length
        Write-Host "  ✓ Screenshot saved: $exeName.png ($([math]::Round($fileSize/1KB, 1)) KB)" -ForegroundColor Green
        $successCount++
    } else {
        Write-Host "  ✗ Failed to capture screenshot" -ForegroundColor Red
        $failCount++
    }
    
    # Close the application
    if (-not $process.HasExited) {
        $process.CloseMainWindow() | Out-Null
        Start-Sleep -Milliseconds 500
        if (-not $process.HasExited) {
            Stop-Process -Id $process.Id -Force
        }
    }
    
    # Small delay between examples
    Start-Sleep -Milliseconds 500
}

Write-Host "`n=== Screenshot Capture Complete ===" -ForegroundColor Green
Write-Host "Successful: $successCount" -ForegroundColor Green
Write-Host "Failed: $failCount" -ForegroundColor $(if ($failCount -gt 0) { "Red" } else { "Green" })
Write-Host "`nScreenshots saved to: $imagesPath" -ForegroundColor Cyan
