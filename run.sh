#!/bin/bash

# YATHA 自动运行脚本
# 自动检测平台、安装 xmake、构建并运行项目

set -e

echo "========================================="
echo "  YATHA 热词统计系统 - 自动运行脚本"
echo "========================================="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检测操作系统
detect_platform() {
    echo -e "${BLUE}[1/4] 检测操作系统平台...${NC}"
    
    case "$(uname -s)" in
        Linux*)     
            PLATFORM="Linux"
            echo -e "${GREEN}✓ 检测到平台: Linux${NC}"
            ;;
        Darwin*)    
            PLATFORM="macOS"
            echo -e "${GREEN}✓ 检测到平台: macOS${NC}"
            ;;
        CYGWIN*|MINGW*|MSYS*)
            PLATFORM="Windows"
            echo -e "${GREEN}✓ 检测到平台: Windows (Git Bash)${NC}"
            ;;
        *)
            echo -e "${RED}✗ 未知平台: $(uname -s)${NC}"
            exit 1
            ;;
    esac
    echo ""
}

# 检查 xmake 是否已安装
check_xmake() {
    if command -v xmake &> /dev/null; then
        XMAKE_VERSION=$(xmake --version 2>&1 | head -n 1)
        return 0
    else
        return 1
    fi
}

# 安装 xmake
install_xmake() {
    echo -e "${BLUE}[2/4] 安装 xmake 构建工具...${NC}"
    echo ""
    
    local install_success=false
    
    # 方式1: 官方安装脚本 (首选)
    echo "方式1: 使用官方安装脚本..."
    if command -v curl &> /dev/null; then
        echo "执行: curl -fsSL https://xmake.io/shget.text | bash"
        if curl -fsSL https://xmake.io/shget.text | bash; then
            install_success=true
        fi
    elif command -v wget &> /dev/null; then
        echo "执行: wget https://xmake.io/shget.text -O - | bash"
        if wget https://xmake.io/shget.text -O - | bash; then
            install_success=true
        fi
    else
        echo -e "${YELLOW}需要 curl 或 wget 来使用官方安装脚本${NC}"
    fi
    
    # 重新加载环境变量
    export PATH="$HOME/.local/bin:$PATH"
    
    # 如果官方脚本失败，尝试备选方式
    if [ "$install_success" = false ]; then
        echo ""
        echo -e "${YELLOW}官方脚本安装失败，尝试备选方式...${NC}"
        echo ""
        
        # 方式2: 包管理器 (备选)
        if [ "$PLATFORM" = "macOS" ] && command -v brew &> /dev/null; then
            echo "方式2: 使用 Homebrew 安装..."
            echo "执行: brew install xmake"
            if brew install xmake; then
                install_success=true
            fi
        elif [ "$PLATFORM" = "Linux" ]; then
            if command -v snap &> /dev/null; then
                echo "方式2: 使用 Snap 安装..."
                echo "执行: sudo snap install xmake --classic"
                if sudo snap install xmake --classic; then
                    install_success=true
                fi
            fi
        fi
    fi
    
    # 重新加载环境变量
    export PATH="$HOME/.local/bin:$PATH"
    
    # 验证安装
    if check_xmake; then
        echo ""
        echo -e "${GREEN}xmake 安装成功: $XMAKE_VERSION${NC}"
    else
        echo ""
        echo -e "${RED}xmake 安装失败${NC}"
        echo ""
        echo -e "${YELLOW}请尝试手动安装:${NC}"
        echo "  官方脚本: curl -fsSL https://xmake.io/shget.text | bash"
        echo "  或访问: https://xmake.io/#/guide/installation"
        if [ "$PLATFORM" = "macOS" ]; then
            echo "  Homebrew: brew install xmake"
        elif [ "$PLATFORM" = "Linux" ]; then
            echo "  Snap: sudo snap install xmake --classic"
        fi
        exit 1
    fi
    echo ""
}



# 运行项目
run_project() {
    echo -e "${BLUE}[4/4] 运行 YATHA...${NC}"
    echo "========================================="
    echo ""
    
    xmake run
}

# 主流程
main() {
    # 检测平台
    detect_platform
    
    # 检查并安装 xmake
    if check_xmake; then
        echo -e "${BLUE}[2/4] 检查 xmake 构建工具...${NC}"
        echo -e "${GREEN}✓ xmake 已安装: $XMAKE_VERSION${NC}"
        echo ""
    else
        echo -e "${BLUE}[2/4] 检查 xmake 构建工具...${NC}"
        echo -e "${YELLOW}✗ xmake 未安装${NC}"
        echo ""
        read -p "是否现在安装 xmake? (y/n): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            install_xmake
        else
            echo -e "${YELLOW}取消安装，退出脚本${NC}"
            exit 0
        fi
    fi
    
    # 运行项目
    run_project
}

# 执行主流程
main "$@"
