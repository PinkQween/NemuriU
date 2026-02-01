cmake_minimum_required(VERSION 3.10)
project(cvn C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Include directories
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Engine library sources
set(ENGINE_SOURCES
    src/cvn_engine.c
    src/engine/window.c
    src/engine/renderer.c
    src/engine/resource.c
    src/engine/audio.c
    src/engine/api.c
)

# Script parser sources
set(PARSER_SOURCES
    src/script/cvn_parser.c
)

# Build static engine library
add_library(cvn_engine STATIC ${ENGINE_SOURCES} ${PARSER_SOURCES})
target_link_libraries(cvn_engine
    ${SDL2_LIBRARIES}
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
)

# Build runtime executable
add_executable(cvn_runtime runtime/main.c)
target_link_libraries(cvn_runtime cvn_engine)

# Optional: Build parser test (legacy)
add_executable(cvn_parser_test main.c src/script/cvn_parser.c src/script/cvn_parser.h)
