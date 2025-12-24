# YATHA Auto Build & Run Script (Windows PowerShell)
# Detects Git Bash and runs run.sh if available

$ErrorActionPreference = "Stop"

# 设置控制台编码为 UTF-8，避免中文乱码和文件读取问题
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::InputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8
chcp 65001 | Out-Null

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  YATHA Auto Build & Run Script" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

function Find-GitBash {
    Write-Host "Checking for Git installation..." -ForegroundColor Blue
    
    # 尝试直接调用 git 命令
    try {
        $gitVersion = & git --version 2>$null
        if ($gitVersion) {
            Write-Host "Git detected: $gitVersion" -ForegroundColor Green
            
            # 尝试找到 git.exe 的路径
            $gitPath = (Get-Command git -ErrorAction SilentlyContinue).Source
            
            if ($gitPath) {
                # 从 git.exe 路径推断 bash.exe 路径
                $gitDir = Split-Path (Split-Path $gitPath -Parent) -Parent
                $bashPath = Join-Path $gitDir "bin\bash.exe"
                
                if (Test-Path $bashPath) {
                    Write-Host "Git Bash found: $bashPath" -ForegroundColor Green
                    return $bashPath
                }
                
                # 尝试另一个可能的路径
                $bashPath = Join-Path $gitDir "usr\bin\bash.exe"
                if (Test-Path $bashPath) {
                    Write-Host "Git Bash found: $bashPath" -ForegroundColor Green
                    return $bashPath
                }
            }
        }
    } catch {
        # git 命令不存在
    }
    
    # 尝试常见的 Git 安装路径
    $commonPaths = @(
        "C:\Program Files\Git\bin\bash.exe",
        "C:\Program Files (x86)\Git\bin\bash.exe",
        "C:\Program Files\Git\usr\bin\bash.exe",
        "$env:LOCALAPPDATA\Programs\Git\bin\bash.exe",
        "$env:LOCALAPPDATA\Programs\Git\usr\bin\bash.exe"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            Write-Host "Git Bash found: $path" -ForegroundColor Green
            return $path
        }
    }
    
    return $null
}

function Run-WithGitBash {
    param (
        [string]$BashPath
    )
    
    Write-Host ""
    Write-Host "Running run.sh with Git Bash..." -ForegroundColor Cyan
    Write-Host "=========================================" -ForegroundColor Cyan
    Write-Host ""
    
    # 获取当前目录并转换为 Unix 风格路径
    $currentDir = (Get-Location).Path
    $unixPath = $currentDir -replace '\\', '/' -replace '^([A-Z]):', { "/$($_.Groups[1].Value.ToLower())" }
    
    # 确保 run.sh 存在
    if (-not (Test-Path "run.sh")) {
        Write-Host "Error: run.sh not found in current directory" -ForegroundColor Red
        exit 1
    }
    
    # 使用 Git Bash 运行 run.sh
    & $BashPath -c "cd '$unixPath' && bash run.sh"
    
    exit $LASTEXITCODE
}

function Show-GitInstallInstructions {
    Write-Host ""
    Write-Host "Git is not installed on this system." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To use this script, please install Git for Windows:" -ForegroundColor Cyan
    Write-Host "  Download: https://git-scm.com/download/win" -ForegroundColor Gray
    Write-Host "  Or use: winget install Git.Git" -ForegroundColor Gray
    Write-Host ""
    Write-Host "After installing Git, run this script again." -ForegroundColor Yellow
    Write-Host ""
    exit 1
}

function Main {
    try {
        # 检测 Git Bash
        $bashPath = Find-GitBash
        
        if ($bashPath) {
            # 找到 Git Bash，使用它运行 run.sh
            Run-WithGitBash -BashPath $bashPath
        } else {
            # 没有找到 Git，显示安装说明
            Show-GitInstallInstructions
        }
        
    } catch {
        Write-Host ""
        Write-Host "Error occurred: $_" -ForegroundColor Red
        Write-Host $_.ScriptStackTrace -ForegroundColor Red
        exit 1
    }
}

Main