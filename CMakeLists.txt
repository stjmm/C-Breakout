cmake_minimum_required(VERSION 3.15)
project(Breakout C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(lib/glfw)

add_library(glad STATIC lib/glad/src/glad.c)
target_include_directories(glad PUBLIC lib/glad/include)

include_directories(lib/stb)
include_directories(lib/linmath)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.c src/*.h)

add_executable(breakout ${SRC_FILES})

target_link_libraries(breakout PRIVATE glad glfw)

target_include_directories(breakout PRIVATE
    lib/glad/include
    lib/glfw/include
)
