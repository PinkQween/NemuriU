#!/usr/bin/env zsh
set -euo pipefail

err() { print -u2 -- "❌ $*"; exit 1; }
info(){ print -- "ℹ️  $*"; }
ok()  { print -- "✅ $*"; }

ROOT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

# Force export the devkitPro paths for this session
export DEVKITPRO=/opt/devkitpro
export PATH="$DEVKITPRO/tools/bin:$PATH"

# 1. Define the correct Wii U CMake wrapper path
# On macOS, this is typically here:
CMAKE_WIIU="/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake"

if [[ ! -x "$CMAKE_WIIU" ]]; then
    err "Wii U CMake wrapper not found at $CMAKE_WIIU. Is wiiu-cmake installed?"
fi

# 2. Clean and Configure
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

info "Configuring for Wii U..."
# Using the wrapper is still best to set the compiler
/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake .. \
    -DSDL2_DIR=/opt/devkitpro/portlibs/wiiu/lib/cmake/SDL2

# 3. Build the .rpx

info "Building..."
cmake --build . -- -j$(sysctl -n hw.ncpu)

# 4. Install (organizes files into a folder structure)
info "Installing to local bundle..."
cmake --install . --prefix "$BUILD_DIR/install"

# 5. Package into .wuhb (Optional, for Aroma environment)
if command -v wuhbtool >/dev/null; then
    info "Packaging .wuhb..."
    wuhbtool "$BUILD_DIR/NemuriU.rpx" "$BUILD_DIR/NemuriU.wuhb" \
      --name="NemuriU" \
      --author="Hanna Skairipa" \
      --icon="$ROOT_DIR/icon.png" \
      --content="$ROOT_DIR/content"
fi

ok "Build Complete! Files are in $BUILD_DIR"