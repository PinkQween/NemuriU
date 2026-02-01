cmake_minimum_required(VERSION 3.10)
project(NemuriU C)

# CVN Engine Integration
set(CVN_ENGINE_DIR "${CMAKE_SOURCE_DIR}/cvn_engine")

# 1. Define all possible lib paths
set(WUT_ROOT "/opt/devkitpro/wut")
set(WIIU_PORTLIBS "/opt/devkitpro/portlibs/wiiu")
set(PPC_PORTLIBS "/opt/devkitpro/portlibs/ppc")
set(TOOLS_BIN "/opt/devkitpro/tools/bin")

# 2. Add BOTH include/lib paths + CVN Engine
include_directories(
    ${WUT_ROOT}/include 
    ${WIIU_PORTLIBS}/include 
    ${PPC_PORTLIBS}/include
    ${CVN_ENGINE_DIR}/include
    ${CVN_ENGINE_DIR}/src
)
link_directories(${WUT_ROOT}/lib ${WIIU_PORTLIBS}/lib ${PPC_PORTLIBS}/lib)

find_package(SDL2 REQUIRED CONFIG)

# CVN Engine sources
set(CVN_SOURCES
    ${CVN_ENGINE_DIR}/src/cvn_engine.c
    ${CVN_ENGINE_DIR}/src/engine/window.c
    ${CVN_ENGINE_DIR}/src/engine/renderer.c
    ${CVN_ENGINE_DIR}/src/engine/resource.c
    ${CVN_ENGINE_DIR}/src/engine/text.c
    ${CVN_ENGINE_DIR}/src/engine/api.c
    ${CVN_ENGINE_DIR}/src/script/cvn_parser.c
)

# Conditional audio - SDL_mixer has issues on Wii U
if(WIIU)
    message(STATUS "Wii U build: Using audio stub (SDL_mixer has dependency issues)")
    list(APPEND CVN_SOURCES ${CVN_ENGINE_DIR}/src/engine/audio_stub.c)
    add_compile_definitions(CVN_AUDIO_STUB)
else()
    message(STATUS "Desktop build: Using SDL_mixer")
    list(APPEND CVN_SOURCES ${CVN_ENGINE_DIR}/src/engine/audio.c)
endif()

add_executable(NemuriU
    src/main.c
    src/nekopara_demo.c
    ${CVN_SOURCES}
)

target_link_libraries(NemuriU PRIVATE
    SDL2::SDL2
    -lSDL2_ttf
    -lSDL2_image
    -lharfbuzz
    -lfreetype
    -ljpeg
    -lpng
    -lz
    -lbz2
    -lwut
    -lm
)

# Only link SDL_mixer on desktop (has issues on Wii U)
if(NOT WIIU)
    target_link_libraries(NemuriU PRIVATE -lSDL2_mixer)
endif()

add_custom_command(TARGET NemuriU POST_BUILD
    COMMAND ${TOOLS_BIN}/elf2rpl $<TARGET_FILE:NemuriU> ${CMAKE_BINARY_DIR}/NemuriU.rpx
    COMMENT "Converting ELF to Wii U RPX..."
)

add_custom_command(TARGET NemuriU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/content
    ${CMAKE_BINARY_DIR}/content
    COMMENT "Copying content directory to build..."
)