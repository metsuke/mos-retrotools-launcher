#!/bin/bash

# setup-retro-x.sh
# Automates setup and launch of Retro-X.exe with Wine on macOS

# Paths
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LAUNCHER_DIR="$SCRIPT_DIR/../.."
LAUNCHER_SCRIPT="$LAUNCHER_DIR/mosRetroLauncher.py"
RETROX_EXE="$LAUNCHER_DIR/assets/bin/retro-x/Retro-X.exe"
CONFIG_DIR="$LAUNCHER_DIR/assets/wine-config"
CONFIG_FILE="$CONFIG_DIR/retro-x.cfg"
WINE_PREFIX="$HOME/.wine-retrox"
LOG_FILE="$WINE_PREFIX/wine-retro-x.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to check command existence
check_command() {
    command -v "$1" >/dev/null 2>&1 || { echo -e "${RED}Error: $1 is not installed.${NC}" >&2; return 1; }
}

# Function to check file existence
check_file() {
    [ -f "$1" ] || { echo -e "${RED}Error: $1 not found.${NC}" >&2; return 1; }
}

# Function to check if process is running
check_process() {
    pgrep -f "$1" >/dev/null 2>&1
}

# Step 1: Check and install Homebrew
echo "Checking for Homebrew..."
if ! check_command brew; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    # Add Homebrew to PATH
    if [ "$(uname -m)" = "arm64" ]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
    else
        eval "$(/usr/local/bin/brew shellenv)"
    fi
    check_command brew || { echo -e "${RED}Failed to install Homebrew.${NC}"; exit 1; }
fi
echo -e "${GREEN}Homebrew is installed.${NC}"

# Step 2: Install Wine and winetricks
echo "Checking for Wine..."
if ! check_command wine; then
    echo "Installing wine-stable..."
    brew install --cask wine-stable
    check_command wine || { echo -e "${RED}Failed to install Wine.${NC}"; exit 1; }
fi
WINE_VERSION=$(wine --version)
echo -e "${GREEN}Wine installed: $WINE_VERSION${NC}"

echo "Checking for winetricks..."
if ! check_command winetricks; then
    echo "Installing winetricks..."
    brew install winetricks
    check_command winetricks || { echo -e "${RED}Failed to install winetricks.${NC}"; exit 1; }
fi
echo -e "${GREEN}winetricks installed.${NC}"

# Step 3: Verify launcher script and Retro-X executable
check_file "$LAUNCHER_SCRIPT" || { echo -e "${RED}Launcher script missing. Please ensure launcher.py is at $LAUNCHER_SCRIPT.${NC}"; exit 1; }
check_file "$RETROX_EXE" || { echo -e "${RED}Retro-X.exe missing. Please ensure it is at $RETROX_EXE.${NC}"; exit 1; }
echo -e "${GREEN}Launcher script and Retro-X.exe found.${NC}"

# Step 4: Create and configure Wine prefix
echo "Setting up Wine prefix at $WINE_PREFIX..."
if [ ! -d "$WINE_PREFIX" ]; then
    WINEPREFIX="$WINE_PREFIX" WINEARCH=win32 winecfg -v win10 >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to create Wine prefix.${NC}"
        exit 1
    fi
    # Configure Virtual Desktop
    WINEPREFIX="$WINE_PREFIX" wine reg add "HKEY_CURRENT_USER\Software\Wine\Explorer" /v Desktop /t REG_SZ /d Default /f >/dev/null 2>&1
    WINEPREFIX="$WINE_PREFIX" wine reg add "HKEY_CURRENT_USER\Software\Wine\Explorer\Desktops" /v Default /t REG_SZ /d 1280x720 /f >/dev/null 2>&1
    echo -e "${GREEN}Wine prefix created and Virtual Desktop set to 1280x720.${NC}"
else
    echo -e "${YELLOW}Wine prefix already exists. Checking configuration...${NC}"
    # Verify Virtual Desktop settings
    if ! grep -q '"Desktop"="Default"' "$WINE_PREFIX/user.reg" || ! grep -q '"Default"="1280x720"' "$WINE_PREFIX/user.reg"; then
        WINEPREFIX="$WINE_PREFIX" wine reg add "HKEY_CURRENT_USER\Software\Wine\Explorer" /v Desktop /t REG_SZ /d Default /f >/dev/null 2>&1
        WINEPREFIX="$WINE_PREFIX" wine reg add "HKEY_CURRENT_USER\Software\Wine\Explorer\Desktops" /v Default /t REG_SZ /d 1280x720 /f >/dev/null 2>&1
        echo -e "${GREEN}Virtual Desktop set to 1280x720.${NC}"
    else
        echo -e "${GREEN}Virtual Desktop already configured.${NC}"
    fi
fi

# Set GDI renderer
echo "Setting GDI renderer..."
WINEPREFIX="$WINE_PREFIX" winetricks -q renderer=gdi >/dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}GDI renderer set.${NC}"
else
    echo -e "${RED}Failed to set GDI renderer.${NC}"
    exit 1
fi

# Step 5: Install dependencies
echo "Installing Wine dependencies..."
WINEPREFIX="$WINE_PREFIX" winetricks -q dotnet48 vcrun2019 gdiplus windowscodecs >/dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Dependencies installed: dotnet48, vcrun2019, gdiplus, windowscodecs.${NC}"
else
    echo -e "${YELLOW}Some dependencies may have failed. Trying fallback \(dotnet20, vcrun2010\)...${NC}"
    WINEPREFIX="$WINE_PREFIX" WINEARCH=win32 winecfg -v winxp >/dev/null 2>&1
    WINEPREFIX="$WINE_PREFIX" winetricks -q dotnet20 vcrun2010 gdiplus windowscodecs >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Fallback dependencies installed.${NC}"
    else
        echo -e "${RED}Failed to install dependencies.${NC}"
        exit 1
    fi
fi

# Step 6: Create retro-x.cfg
echo "Creating retro-x.cfg..."
mkdir -p "$CONFIG_DIR"
cat > "$CONFIG_FILE" << 'EOF'
# Wine configuration for Retro-X
# Debug GUI and GDI issues
WINEDEBUG=+err,+warn,+win,+gdi
# Dedicated Wine prefix
WINEPREFIX=~/.wine-retrox
# Force 32-bit mode (based on WoW64 log)
WINEARCH=win32
EOF
if [ -f "$CONFIG_FILE" ]; then
    echo -e "${GREEN}retro-x.cfg created at $CONFIG_FILE.${NC}"
else
    echo -e "${RED}Failed to create retro-x.cfg.${NC}"
    exit 1
fi

# Step 7: Launch the launcher
echo "Launching Retro-X launcher..."
cd "$LAUNCHER_DIR" || { echo -e "${RED}Failed to change to $LAUNCHER_DIR.${NC}"; exit 1; }
python3 "$LAUNCHER_SCRIPT" &
LAUNCHER_PID=$!
echo -e "${GREEN}Launcher started \(PID: $LAUNCHER_PID\). Please click the Retro-X button.${NC}"

# Wait for Wine process to start (up to 30 seconds)
echo "Waiting for Retro-X to launch..."
sleep 5
for i in {1..25}; do
    if check_process "Retro-X.exe"; then
        echo -e "${GREEN}Retro-X.exe is running. Check for a Virtual Desktop window \(1280x720\).${NC}"
        exit 0
    fi
    sleep 1
done

# Step 8: If no Wine process, collect diagnostics
echo -e "${RED}Retro-X.exe did not start.${NC}"
if [ -f "$LOG_FILE" ]; then
    echo -e "${YELLOW}Wine log found at $LOG_FILE. Last 20 lines:${NC}"
    tail -n 20 "$LOG_FILE"
else
    echo -e "${YELLOW}No Wine log found at $LOG_FILE. Running manual test...${NC}"
    WINEDEBUG=+err,+warn,+win,+gdi WINEPREFIX="$WINE_PREFIX" WINEARCH=win32 wine "$RETROX_EXE" 2>&1 | tee "$LAUNCHER_DIR/retro-x-log.txt"
    echo -e "${YELLOW}Manual test log saved at $LAUNCHER_DIR/retro-x-log.txt.${NC}"
fi

echo -e "${YELLOW}Next steps:${NC}"
echo "1. Check for a Wine Virtual Desktop window \(1280x720\). If minimized, use Alt+Tab."
echo "2. Review the log file \($LOG_FILE or $LAUNCHER_DIR/retro-x-log.txt\) for errors."
echo "3. Verify Wine version: $(wine --version). Use Wine 9.0+ \(brew install --cask wine-stable\)."
echo "4. Try CrossOver \(codeweavers.com\) or Whisky \(brew install --cask whisky\)."
echo "5. For native conversion, use ImageMagick: convert input.png -colors 256 output.bmp"
exit 1