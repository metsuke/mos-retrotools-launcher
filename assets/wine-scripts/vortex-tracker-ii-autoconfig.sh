#!/bin/bash

# setup-vortex-tracker-ii.sh
# Automates setup and launch of Vortex Tracker II with Wine on macOS

# Paths
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LAUNCHER_DIR="$SCRIPT_DIR/../.."
VORTEX_EXE="$LAUNCHER_DIR/assets/bin/vortex-tracker-ii/VT.exe"
CONFIG_DIR="$LAUNCHER_DIR/assets/wine-config"
CONFIG_FILE="$CONFIG_DIR/vortex-tracker-ii.cfg"
WINE_PREFIX="$HOME/.wine-vortex"
LOG_FILE="$WINE_PREFIX/wine-vortex-tracker-ii.log"

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

# Step 3: Verify Vortex Tracker II executable
check_file "$VORTEX_EXE" || { echo -e "${RED}VortexTrackerII.exe missing. Please ensure it is at $VORTEX_EXE.${NC}"; exit 1; }
echo -e "${GREEN}Vortex Tracker II executable found.${NC}"

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

# Step 5: Install dependencies for Vortex Tracker II
echo "Installing Wine dependencies..."
WINEPREFIX="$WINE_PREFIX" winetricks -q vcrun2010 gdiplus mfc42 dsound >/dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Dependencies installed: vcrun2010, gdiplus, mfc42, dsound.${NC}"
else
    echo -e "${YELLOW}Some dependencies may have failed. Trying fallback...${NC}"
    WINEPREFIX="$WINE_PREFIX" WINEARCH=win32 winecfg -v winxp >/dev/null 2>&1
    WINEPREFIX="$WINE_PREFIX" winetricks -q vcrun2005 gdiplus mfc42 dsound >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Fallback dependencies installed.${NC}"
    else
        echo -e "${RED}Failed to install dependencies.${NC}"
        exit 1
    fi
fi

# Step 6: Create vortex-tracker-ii surrender.cfg
echo "Creating vortex-tracker-ii.cfg..."
mkdir -p "$CONFIG_DIR"
cat > "$CONFIG_FILE" << 'EOF'
# Wine configuration for Vortex Tracker II
# Debug audio and GUI issues
WINEDEBUG=+err,+warn,+win,+gdi,+dsound
# Dedicated Wine prefix
WINEPREFIX=$HOME/.wine-vortex
# Force 32-bit mode
WINEARCH=win32
EOF
if [ -f "$CONFIG_FILE" ]; then
    echo -e "${GREEN}vortex-tracker-ii.cfg created at $CONFIG_FILE.${NC}"
else
    echo -e "${RED}Failed to create vortex-tracker-ii.cfg.${NC}"
    exit 1
fi

# Step 7: Read and process configuration file (replacing $HOME)
echo "Processing configuration file..."
if [ -f "$CONFIG_FILE" ]; then
    # Read config file, replace $HOME with actual HOME value, exclude comments and empty lines
    config_lines=()
    while IFS= read -r line; do
        # Skip empty lines and comments
        if [[ -n "$line" && ! "$line" =~ ^# ]]; then
            # Replace $HOME with the value of HOME environment variable
            processed_line=$(echo "$line" | envsubst '$HOME')
            config_lines+=("$processed_line")
        fi
    done < "$CONFIG_FILE"
    echo -e "${GREEN}Processed configuration: ${config_lines[*]}${NC}"
else
    echo -e "${RED}Configuration file $CONFIG_FILE not found.${NC}"
    exit 1
fi

# Step 8: Launch Vortex Tracker II
echo "Launching Vortex Tracker II..."
cd "$LAUNCHER_DIR" || { echo -e "${RED}Failed to change to $LAUNCHER_DIR.${NC}"; exit 1; }
# Apply configuration from processed lines
for line in "${config_lines[@]}"; do
    eval "export $line"
done
WINEPREFIX="$WINE_PREFIX" WINEARCH=win32 wine "$VORTEX_EXE" 2>&1 | tee "$LOG_FILE" &
VORTEX_PID=$!
echo -e "${GREEN}Vortex Tracker II started (PID: $VORTEX_PID).${NC}"

# Wait for Wine process to start (up to 30 seconds)
echo "Waiting for Vortex Tracker II to launch..."
sleep 5
for i in {1..25}; do
    if check_process "VT.exe"; then
        echo -e "${GREEN}VortexTrackerII.exe is running. Check for a Virtual Desktop window (1280x720).${NC}"
        exit 0
    fi
    sleep 1
done

# Step 9: If no Wine process, collect diagnostics
echo -e "${RED}VortexTrackerII.exe did not start.${NC}"
if [ -f "$LOG_FILE" ]; then
    echo -e "${YELLOW}Wine log found at $LOG_FILE. Last 20 lines:${NC}"
    tail -n 20 "$LOG_FILE"
else
    echo -e "${YELLOW}No Wine log found at $