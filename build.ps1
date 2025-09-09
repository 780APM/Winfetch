# PowerShell build script for Winfetch
Write-Host "Building Winfetch..." -ForegroundColor Green

# Try to find Visual Studio installation
$vsPaths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

$vsPath = $null
foreach ($path in $vsPaths) {
    if (Test-Path $path) {
        $vsPath = $path
        break
    }
}

if (-not $vsPath) {
    Write-Host "Visual Studio not found! Please install Visual Studio with C++ support." -ForegroundColor Red
    Write-Host "Download from: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Yellow

# Create output directory
if (-not (Test-Path "bin")) {
    New-Item -ItemType Directory -Name "bin" | Out-Null
}

# Set up Visual Studio environment and compile
$tempBat = "temp_build.bat"
@"
@call "$vsPath"
cl /EHsc /I include /Fe:bin\winfetch6.exe src\main.cpp src\system_info.cpp src\display.cpp src\config.cpp src\ascii_art.cpp /link kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib psapi.lib powrprof.lib wbemuuid.lib ws2_32.lib
"@ | Out-File -FilePath $tempBat -Encoding ASCII

try {
    & cmd /c $tempBat
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`nBuild successful!" -ForegroundColor Green
        Write-Host "Executable is located at: bin\winfetch6.exe" -ForegroundColor Cyan
        Write-Host "`nYou can now run: .\bin\winfetch6.exe" -ForegroundColor Yellow
    } else {
        Write-Host "`nBuild failed!" -ForegroundColor Red
    }
} finally {
    # Clean up temp file
    if (Test-Path $tempBat) {
        Remove-Item $tempBat
    }
}

Read-Host "Press Enter to exit"
