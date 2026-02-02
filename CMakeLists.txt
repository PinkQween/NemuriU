
cmake_minimum_required(VERSION 3.10)
project(NemuriU C)

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "WiiU" OR DEFINED WIIU OR DEFINED __WIIU__)
    set(BUILD_WIIU TRUE)
    message(STATUS "Configuring for Wii U")
else()
    set(BUILD_WIIU FALSE)
    message(STATUS "Configuring for Desktop (Linux/macOS)")
endif()

# CVN Engine Integration
set(CVN_ENGINE_DIR "${CMAKE_SOURCE_DIR}/cvn")


if(BUILD_WIIU)
    # Wii U toolchain and libraries
    set(WUT_ROOT "/opt/devkitpro/wut")
    set(WIIU_PORTLIBS "/opt/devkitpro/portlibs/wiiu")
    set(PPC_PORTLIBS "/opt/devkitpro/portlibs/ppc")
    set(TOOLS_BIN "/opt/devkitpro/tools/bin")
    include_directories(
        ${WUT_ROOT}/include
        ${WIIU_PORTLIBS}/include
        ${PPC_PORTLIBS}/include
        ${CVN_ENGINE_DIR}/include
        ${CVN_ENGINE_DIR}/src
    )
    link_directories(${WUT_ROOT}/lib ${WIIU_PORTLIBS}/lib ${PPC_PORTLIBS}/lib)
    find_package(SDL2 REQUIRED CONFIG)
else()
    # Desktop (Linux/macOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
    include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
        ${CVN_ENGINE_DIR}/include
        ${CVN_ENGINE_DIR}/src
    )
    link_directories(
        ${SDL2_LIBRARY_DIRS}
        ${SDL2_TTF_LIBRARY_DIRS}
        ${SDL2_IMAGE_LIBRARY_DIRS}
        ${SDL2_MIXER_LIBRARY_DIRS}
    )
endif()

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
if(BUILD_WIIU)
    message(STATUS "Wii U build: Using audio stub (SDL_mixer has dependency issues)")
    list(APPEND CVN_SOURCES ${CVN_ENGINE_DIR}/src/engine/audio_stub.c)
    add_compile_definitions(CVN_AUDIO_STUB)
    add_compile_definitions(WIIU)
else()
    message(STATUS "Desktop build: Using SDL_mixer")
    list(APPEND CVN_SOURCES ${CVN_ENGINE_DIR}/src/engine/audio.c)
endif()


add_executable(NemuriU
    src/main.c
    ${CVN_SOURCES}
)


if(BUILD_WIIU)
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
else()
    target_link_libraries(NemuriU PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        harfbuzz
        freetype
        jpeg
        png
        z
        bz2
        m
    )
    if(APPLE)
        target_link_libraries(NemuriU PRIVATE "-framework Cocoa")
    endif()
endif()


# Wii U only: Convert ELF to RPX and copy content
if(BUILD_WIIU)
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
else()
    # Desktop: just copy content
    add_custom_command(TARGET NemuriU POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/content
        ${CMAKE_BINARY_DIR}/content
        COMMENT "Copying content directory to build..."
    )
endif()