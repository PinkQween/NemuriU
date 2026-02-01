cmake_minimum_required(VERSION 3.10)
project(NemuriU C)

# 1. Paths
set(WIIU_PORTLIBS "/opt/devkitpro/portlibs/wiiu")
set(WUT_ROOT "/opt/devkitpro/wut")
set(TOOLS_BIN "/opt/devkitpro/tools/bin")

include_directories(${WUT_ROOT}/include ${WIIU_PORTLIBS}/include)
link_directories(${WUT_ROOT}/lib)

find_package(SDL2 REQUIRED CONFIG)

add_executable(NemuriU src/main.c)

target_link_libraries(NemuriU PRIVATE 
    SDL2::SDL2
    -lwut
    -lm
)

# 2. Use elf2rpl (The new tool found in your bin)
add_custom_command(TARGET NemuriU POST_BUILD
    COMMAND ${TOOLS_BIN}/elf2rpl $<TARGET_FILE:NemuriU> ${CMAKE_BINARY_DIR}/NemuriU.rpx
    COMMENT "Converting ELF to Wii U RPX using elf2rpl..."
)
