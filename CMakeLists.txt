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
    src/engine/Texture.cpp
    src/engine/Texture.h
    src/engine/Renderer2D.cpp
    src/engine/Renderer2D.h
)
if (MSVC)
    target_compile_options(erlik PRIVATE /W4 /permissive- /Zc:preprocessor)
endif()
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
target_link_libraries(erlik PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)
target_include_directories(erlik PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
