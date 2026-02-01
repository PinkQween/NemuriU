cmake_minimum_required(VERSION 3.10)

# Wii U projects MUST be C or C++
project(NemuriU C)

# -------------------------------------------------
# Use devkitPro Wii U toolchain (cmake-wiiu does this)
# -------------------------------------------------
find_package(PkgConfig REQUIRED)

# -------------------------------------------------
# SDL2 (Wii U build from devkitPro)
# -------------------------------------------------
pkg_check_modules(SDL2 REQUIRED sdl2)

# -------------------------------------------------
# Executable
# -------------------------------------------------
add_executable(NemuriU
    src/main.c
)

target_include_directories(NemuriU PRIVATE
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(NemuriU PRIVATE
    ${SDL2_LIBRARIES}
)

target_compile_options(NemuriU PRIVATE
    ${SDL2_CFLAGS_OTHER}
)

# -------------------------------------------------
# Convert ELF → RPX (WUT helper)
# -------------------------------------------------
wut_create_rpx(NemuriU)

# -------------------------------------------------
# Install into .app directory
# -------------------------------------------------
set(APP_DIR "${CMAKE_BINARY_DIR}/NemuriU.app")

install(TARGETS NemuriU
    RUNTIME DESTINATION ${APP_DIR}
)