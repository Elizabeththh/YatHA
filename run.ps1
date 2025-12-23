# YATHA 自动运行脚本 (Windows PowerShell)
# 自动检测平台、安装 xmake、构建并运行项目

$ErrorActionPreference = "Stop"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  YATHA 热词统计系统 - 自动运行脚本" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# 检测操作系统
function Detect-Platform {
    Write-Host "[1/4] 检测操作系统平台..." -ForegroundColor Blue
    
    $platform = [System.Environment]::OSVersion.Platform
    if ($platform -eq "Win32NT") {
        Write-Host "✓ 检测到平台: Windows $([System.Environment]::OSVersion.Version)" -ForegroundColor Green
        Write-Host ""
        return "Windows"
    } else {
        Write-Host "✗ 未知平台: $platform" -ForegroundColor Red
        Write-Host "  请使用 Linux/macOS 的 run.sh 脚本" -ForegroundColor Yellow
        exit 1
    }
}

# 检查 xmake 是否已安装
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

# 安装 xmake
function Install-Xmake {
    Write-Host "[2/4] 安装 xmake 构建工具..." -ForegroundColor Blue
    Write-Host ""
    
    $installSuccess = $false
    
    # 方式1: 官方 PowerShell 安装脚本 (首选)
    Write-Host "方式1: 使用官方 PowerShell 脚本..." -ForegroundColor Cyan
    Write-Host "执行: irm https://xmake.io/psget.text | iex" -ForegroundColor Gray
    try {
        Invoke-Expression (Invoke-RestMethod 'https://xmake.io/psget.text')
        
        # 刷新环境变量
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        
        if (Check-Xmake) {
            $installSuccess = $true
            Write-Host "" 
            Write-Host "xmake 安装成功: $script:XmakeVersion" -ForegroundColor Green
        }
    } catch {
        Write-Host "官方脚本安装失败: $_" -ForegroundColor Yellow
    }
    
    # 如果官方脚本失败，尝试备选方式
    if (-not $installSuccess) {
        Write-Host ""
        Write-Host "尝试备选安装方式..." -ForegroundColor Yellow
        Write-Host ""
        
        # 方式2: Scoop (备选)
        try {
            $scoopVersion = & scoop --version 2>$null
            if ($scoopVersion) {
                Write-Host "方式2: 使用 Scoop 安装..." -ForegroundColor Cyan
                Write-Host "执行: scoop install xmake" -ForegroundColor Gray
                & scoop install xmake
                
                if (Check-Xmake) {
                    $installSuccess = $true
                    Write-Host ""
                    Write-Host "xmake 安装成功: $script:XmakeVersion" -ForegroundColor Green
                }
            }
        } catch {
            Write-Host "Scoop 安装失败" -ForegroundColor Yellow
        }
        
        # 方式3: Winget (备选)
        if (-not $installSuccess) {
            try {
                $wingetVersion = & winget --version 2>$null
                if ($wingetVersion) {
                    Write-Host "方式3: 使用 Winget 安装..." -ForegroundColor Cyan
                    Write-Host "执行: winget install xmake" -ForegroundColor Gray
                    & winget install xmake --silent
                    
                    # 刷新环境变量
                    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
                    
                    if (Check-Xmake) {
                        $installSuccess = $true
                        Write-Host ""
                        Write-Host "xmake 安装成功: $script:XmakeVersion" -ForegroundColor Green
                    }
                }
            } catch {
                Write-Host "Winget 安装失败" -ForegroundColor Yellow
            }
        }
    }
    
    # 验证最终安装结果
    if (-not $installSuccess) {
        Write-Host ""
        Write-Host "xmake 自动安装失败" -ForegroundColor Red
        Write-Host ""
        Write-Host "请尝试手动安装:" -ForegroundColor Yellow
        Write-Host "  官方脚本: irm https://xmake.io/psget.text | iex" -ForegroundColor Gray
        Write-Host "  GitHub 下载: https://github.com/xmake-io/xmake/releases" -ForegroundColor Gray
        Write-Host "  Scoop: scoop install xmake" -ForegroundColor Gray
        Write-Host "  Winget: winget install xmake" -ForegroundColor Gray
        Write-Host "  安装指南: https://xmake.io/#/guide/installation" -ForegroundColor Gray
        exit 1
    }
    Write-Host ""
}

# 构建项目
function Build-Project {
    Write-Host "[3/4] 构建项目..." -ForegroundColor Blue
    Write-Host ""
    
    # 配置项目
    Write-Host "配置项目..." -ForegroundColor Cyan
    & xmake config -y
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ 项目配置失败" -ForegroundColor Red
        exit 1
    }
    
    # 编译项目
    Write-Host ""
    Write-Host "编译中..." -ForegroundColor Cyan
    & xmake build -y
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ 项目构建失败" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "✓ 项目构建成功" -ForegroundColor Green
    Write-Host ""
}

# 运行项目
function Run-Project {
    Write-Host "[4/4] 运行 YATHA..." -ForegroundColor Blue
    Write-Host "=========================================" -ForegroundColor Cyan
    Write-Host ""
    
    & xmake run yatha
}

# 主流程
function Main {
    try {
        # 检测平台
        $platform = Detect-Platform
        
        # 检查并安装 xmake
        Write-Host "[2/4] 检查 xmake 构建工具..." -ForegroundColor Blue
        if (Check-Xmake) {
            Write-Host "✓ xmake 已安装: $script:XmakeVersion" -ForegroundColor Green
            Write-Host ""
        } else {
            Write-Host "✗ xmake 未安装" -ForegroundColor Yellow
            Write-Host ""
            
            $response = Read-Host "是否现在安装 xmake? (Y/N)"
            if ($response -match "^[Yy]$") {
                Install-Xmake
            } else {
                Write-Host "取消安装，退出脚本" -ForegroundColor Yellow
                exit 0
            }
        }
        
        # 构建项目
        Build-Project
        
        # 运行项目
        Run-Project
        
    } catch {
        Write-Host ""
        Write-Host "✗ 发生错误: $_" -ForegroundColor Red
        Write-Host $_.ScriptStackTrace -ForegroundColor Red
        exit 1
    }
}

# 执行主流程
Main
