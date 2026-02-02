#!/usr/bin/env zsh
set -euo pipefail

err() { print -u2 -- "❌ $*"; exit 1; }
info(){ print -- "ℹ️  $*"; }
ok()  { print -- "✅ $*"; }

ROOT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"


# Helper function for each build
build_platform() {
    local platform="$1"
    local build_dir="$ROOT_DIR/build/$platform"
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    cd "$build_dir"
    if [[ "$platform" == "wiiu" ]]; then
        export DEVKITPRO=/opt/devkitpro
        export PATH="/opt/devkitpro/tools/bin:$PATH"
        CMAKE_WIIU="/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake"
        if [[ ! -x "$CMAKE_WIIU" ]]; then
            err "Wii U CMake wrapper not found at $CMAKE_WIIU. Is wiiu-cmake installed?"
        fi
        info "Configuring for Wii U..."
        "$CMAKE_WIIU" ../.. -DSDL2_DIR=/opt/devkitpro/portlibs/wiiu/lib/cmake/SDL2
        info "Building..."
        cmake --build . -- -j$(sysctl -n hw.ncpu)
        info "Installing to local bundle..."
        cmake --install . --prefix "$build_dir/install"
        if command -v wuhbtool >/dev/null; then
            info "Packaging .wuhb..."
            wuhbtool "$build_dir/NemuriU.rpx" "$build_dir/NemuriU.wuhb" \
              --name="NemuriU" \
              --author="Hanna Skairipa" \
              --icon="$ROOT_DIR/icon.png" \
              --content="$ROOT_DIR/content"
        fi
        ok "Wii U build complete! Files are in $build_dir"
    elif [[ "$platform" == "linux" ]]; then
        export PATH="/usr/bin:$PATH"
        unset PKG_CONFIG_PATH
        info "Configuring for Linux..."
        cmake ../..
        info "Building..."
        cmake --build . -- -j$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc || sysctl -n hw.ncpu)
        info "Installing to local bundle..."
        cmake --install . --prefix "$build_dir/install"
        ok "Linux build complete! Files are in $build_dir"
    elif [[ "$platform" == "macos" ]]; then
        export PATH="/opt/homebrew/bin:$PATH"
        export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig"
        info "Configuring for macOS..."
        cmake ../..
        info "Building..."
        cmake --build . -- -j$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)
        info "Installing to local bundle..."
        cmake --install . --prefix "$build_dir/install"
        ok "macOS build complete! Files are in $build_dir"
    else
        err "Unknown or unsupported platform: $platform."
    fi
    cd "$ROOT_DIR"
}

# If a platform is specified, build only that
if [[ $# -ge 1 ]]; then
    build_platform "$1"
else
    # Default: build all three
    build_platform macos
    build_platform linux
    build_platform wiiu
fi