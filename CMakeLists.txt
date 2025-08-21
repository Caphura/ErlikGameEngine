cmake_minimum_required(VERSION 3.21)
project(ErlikGameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(erlik
    src/main.cpp
    src/engine/Application.cpp
    src/engine/Application.h
    src/engine/Input.cpp
    src/engine/Input.h
)

# Warnings and sane defaults on MSVC
if (MSVC)
    target_compile_options(erlik PRIVATE /W4 /permissive- /Zc:preprocessor)
endif()

# Find SDL2 via vcpkg (recommended)
# Make sure you have vcpkg installed and integrated (see README).
find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(erlik PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_include_directories(erlik PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)