# YATHA Auto Build & Run Script (Windows PowerShell)
# Auto-detect platform, install xmake, build and run project

$ErrorActionPreference = "Stop"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  YATHA Auto Build & Run Script" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

function Detect-Platform {
    Write-Host "[1/4] Detecting platform..." -ForegroundColor Blue
    
    $platform = [System.Environment]::OSVersion.Platform
    if ($platform -eq "Win32NT") {
        Write-Host "Platform detected: Windows $([System.Environment]::OSVersion.Version)" -ForegroundColor Green
        Write-Host ""
        return "Windows"
    } else {
        Write-Host "Unknown platform: $platform" -ForegroundColor Red
        Write-Host "  Please use run.sh for Linux/macOS" -ForegroundColor Yellow
        exit 1
    }
}

function Check-Xmake {
    try {
        $version = & xmake --version 2>$null | Select-Object -First 1
        if ($version) {
            $script:XmakeVersion = $version
            return $true
        }
    } catch {
        return $false
    }
    return $false
}

function Install-Xmake {
    Write-Host "[2/4] Installing xmake build tool..." -ForegroundColor Blue
    Write-Host ""
    
    $installSuccess = $false
    
    Write-Host "Method 1: Official PowerShell script..." -ForegroundColor Cyan
    Write-Host "Executing: irm https://xmake.io/psget.text | iex" -ForegroundColor Gray
    try {
        Invoke-Expression (Invoke-RestMethod 'https://xmake.io/psget.text')
        
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        
        if (Check-Xmake) {
            $installSuccess = $true
            Write-Host "" 
            Write-Host "xmake installed successfully: $script:XmakeVersion" -ForegroundColor Green
        }
    } catch {
        Write-Host "Official script installation failed: $_" -ForegroundColor Yellow
    }
    
    if (-not $installSuccess) {
        Write-Host ""
        Write-Host "Trying alternative methods..." -ForegroundColor Yellow
        Write-Host ""
        
        try {
            $scoopVersion = & scoop --version 2>$null
            if ($scoopVersion) {
                Write-Host "Method 2: Using Scoop..." -ForegroundColor Cyan
                Write-Host "Executing: scoop install xmake" -ForegroundColor Gray
                & scoop install xmake
                
                if (Check-Xmake) {
                    $installSuccess = $true
                    Write-Host ""
                    Write-Host "xmake installed successfully: $script:XmakeVersion" -ForegroundColor Green
                }
            }
        } catch {
            Write-Host "Scoop installation failed" -ForegroundColor Yellow
        }
        
        if (-not $installSuccess) {
            try {
                $wingetVersion = & winget --version 2>$null
                if ($wingetVersion) {
                    Write-Host "Method 3: Using Winget..." -ForegroundColor Cyan
                    Write-Host "Executing: winget install xmake" -ForegroundColor Gray
                    & winget install xmake --silent
                    
                    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
                    
                    if (Check-Xmake) {
                        $installSuccess = $true
                        Write-Host ""
                        Write-Host "xmake installed successfully: $script:XmakeVersion" -ForegroundColor Green
                    }
                }
            } catch {
                Write-Host "Winget installation failed" -ForegroundColor Yellow
            }
        }
    }
    
    if (-not $installSuccess) {
        Write-Host ""
        Write-Host "xmake auto installation failed" -ForegroundColor Red
        Write-Host ""
        Write-Host "Please install manually:" -ForegroundColor Yellow
        Write-Host "  Official: irm https://xmake.io/psget.text | iex" -ForegroundColor Gray
        Write-Host "  GitHub: https://github.com/xmake-io/xmake/releases" -ForegroundColor Gray
        Write-Host "  Scoop: scoop install xmake" -ForegroundColor Gray
        Write-Host "  Winget: winget install xmake" -ForegroundColor Gray
        Write-Host "  Guide: https://xmake.io/#/guide/installation" -ForegroundColor Gray
        exit 1
    }
    Write-Host ""
}

function Build-Project {
    Write-Host "[3/4] Building project..." -ForegroundColor Blue
    Write-Host ""
    
    Write-Host "Configuring..." -ForegroundColor Cyan
    & xmake config -y
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Configuration failed" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Compiling..." -ForegroundColor Cyan
    & xmake build -y
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Build successful" -ForegroundColor Green
    Write-Host ""
    
    Write-Host "Checking environment..." -ForegroundColor Cyan
    $requiredDirs = @("data\temp", "data\temp\sse", "data\dict")
    foreach ($dir in $requiredDirs) {
        if (-not (Test-Path $dir)) {
            Write-Host "Creating directory: $dir" -ForegroundColor Yellow
            New-Item -ItemType Directory -Path $dir -Force | Out-Null
        } else {
            Write-Host "Directory exists: $dir" -ForegroundColor Green
        }
    }
    
    Write-Host ""
    Write-Host "Checking required files..." -ForegroundColor Cyan
    $requiredPaths = @("web\index.html", "data\dict\jieba.dict.utf8")
    $missingFiles = @()
    foreach ($path in $requiredPaths) {
        if (-not (Test-Path $path)) {
            Write-Host "Missing file: $path" -ForegroundColor Red
            $missingFiles += $path
        } else {
            Write-Host "File exists: $path" -ForegroundColor Green
        }
    }
    
    if ($missingFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "Warning: Missing files detected, program may not work properly" -ForegroundColor Yellow
    }
    
    Write-Host ""
}

function Run-Project {
    Write-Host "[4/4] Running YATHA..." -ForegroundColor Blue
    Write-Host "=========================================" -ForegroundColor Cyan
    Write-Host ""
    
    & xmake run yatha
}

function Main {
    try {
        $platform = Detect-Platform
        
        Write-Host "[2/4] Checking xmake build tool..." -ForegroundColor Blue
        if (Check-Xmake) {
            Write-Host "xmake is installed: $script:XmakeVersion" -ForegroundColor Green
            Write-Host ""
        } else {
            Write-Host "xmake is not installed" -ForegroundColor Yellow
            Write-Host ""
            
            $response = Read-Host "Install xmake now? (Y/N)"
            if ($response -match "^[Yy]$") {
                Install-Xmake
            } else {
                Write-Host "Installation cancelled, exiting" -ForegroundColor Yellow
                exit 0
            }
        }
        
        Build-Project
        Run-Project
        
    } catch {
        Write-Host ""
        Write-Host "Error occurred: $_" -ForegroundColor Red
        Write-Host $_.ScriptStackTrace -ForegroundColor Red
        exit 1
    }
}

Main