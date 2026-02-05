cmake_minimum_required(VERSION 3.16)
project(gf2d C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_compile_options(
        -Wall
        -pedantic
        -fgnu89-inline
        -Wno-unknown-pragmas
        -Wno-variadic-macros
        -Wformat-truncation=0
)

# GF2D Executable
file(GLOB_RECURSE GF2D_SOURCES
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

set(GF2D_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/gfc/include
        ${CMAKE_CURRENT_SOURCE_DIR}/gfc/simple_logger/include
        ${CMAKE_CURRENT_SOURCE_DIR}/gfc/simple_json/include
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_library(gfc STATIC IMPORTED)
set_target_properties(gfc PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/gfc/libs/libgfc.a
)

add_library(simple_json STATIC IMPORTED)
set_target_properties(simple_json PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/gfc/simple_json/libs/libsj.a
)

add_library(simple_logger STATIC IMPORTED)
set_target_properties(simple_logger PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/gfc/simple_logger/libs/libsl.a
)

include(FetchContent)

FetchContent_Declare(
        enet
        GIT_REPOSITORY https://github.com/lsalzman/enet.git
        GIT_TAG v1.3.18
)

FetchContent_MakeAvailable(enet)

# ------------------------------------------------------------------------------
# Static library
# ------------------------------------------------------------------------------
add_executable(gf2d ${GF2D_SOURCES})

target_include_directories(gf2d PRIVATE ${GF2D_INCLUDE_DIRS})

target_link_libraries(gf2d PRIVATE
        gfc
        simple_json
        simple_logger
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        SDL2_mixer::SDL2_mixer
        enet
        pthread
        m
)

target_compile_options(gf2d PRIVATE
        $<$<CONFIG:Debug>:-g3 -O0 -Wall -Wextra>
)