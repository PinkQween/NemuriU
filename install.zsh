#!/usr/bin/env zsh
set -euo pipefail

err() { print -u2 -- "❌ $*"; exit 1; }
info(){ print -- "ℹ️  $*"; }
ok()  { print -- "✅ $*"; }

ROOT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"

# Load .env
ENV_FILE="$ROOT_DIR/.env"
[[ -f "$ENV_FILE" ]] || err ".env not found at: $ENV_FILE"
set -a; source "$ENV_FILE"; set +a
: "${WIIU_IP:?WIIU_IP not set in .env}"

# Tools
CMAKE_WIIU="${CMAKE_WIIU:-/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake}"
command -v "$CMAKE_WIIU" >/dev/null 2>&1 || err "Wii U CMake wrapper not found: $CMAKE_WIIU"
command -v cmake >/dev/null 2>&1 || err "cmake not found"
command -v wuhbtool >/dev/null 2>&1 || err "wuhbtool not found"
command -v ftp >/dev/null 2>&1 || err "ftp not found"

# Build
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

info "Configuring..."
"$CMAKE_WIIU" ..

info "Building..."
cmake --build . -- -j"${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

info "Installing..."
cmake --install .

# Package
APP_DIR="$BUILD_DIR/NemuriU.app"
ELF_PATH="$APP_DIR/NemuriU.elf"
RPX_PATH="$APP_DIR/NemuriU.rpx"   # if your toolchain generates it
[[ -d "$APP_DIR" ]] || err "App dir not found: $APP_DIR"

BIN_PATH=""
if [[ -f "$RPX_PATH" ]]; then
  BIN_PATH="$RPX_PATH"
elif [[ -f "$ELF_PATH" ]]; then
  BIN_PATH="$ELF_PATH"
else
  err "No RPX/ELF found in $APP_DIR (expected NemuriU.rpx or NemuriU.elf)"
fi

NAME="${NAME:-NemuriU}"
SHORT_NAME="${SHORT_NAME:-NemuriU}"
AUTHOR="${AUTHOR:-Hanna Skairipa}"
ICON_PATH="${ICON_PATH:-$ROOT_DIR/icon.png}"

[[ -f "$ICON_PATH" ]] || err "Icon not found: $ICON_PATH"

OUT_FILE="${OUT_FILE:-$BUILD_DIR/NemuriU.wuhb}"

info "Packaging with wuhbtool..."
wuhbtool "$BIN_PATH" "$OUT_FILE" \
  --content="$APP_DIR" \
  --name="$NAME" \
  --short-name="$SHORT_NAME" \
  --author="$AUTHOR" \
  --icon="$ICON_PATH"

[[ -f "$OUT_FILE" ]] || err "Packaging failed: $OUT_FILE was not created"

# Upload
REMOTE_DIR="${REMOTE_DIR:-/fs/vol/external01/wiiu/apps}"
# REMOTE_DIR="${REMOTE_DIR:-/fs/vol/external01/install}"
REMOTE_NAME="${REMOTE_NAME:-NemuriU.wuhb}"

info "Uploading $(basename "$OUT_FILE") to $WIIU_IP:$REMOTE_DIR/$REMOTE_NAME ..."
ftp -n "$WIIU_IP" <<EOF
user anonymous
binary
cd $REMOTE_DIR
put $OUT_FILE $REMOTE_NAME
quit
EOF

ok "Done"
