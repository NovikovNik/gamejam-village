cmake_minimum_required(VERSION 3.16)
project(GameProject CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O1")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SDLTTF_VENDORED ON)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/SDL EXCLUDE_FROM_ALL)

set(SDLTTF_USE_SYSTEM_FREETYPE OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/SDL_ttf EXCLUDE_FROM_ALL)
set(SDLIMAGE_AVIF OFF CACHE BOOL "Support loading AVIF images" FORCE)
add_subdirectory(libs/SDL_image EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(game_engine ${SOURCES})

target_include_directories(game_engine PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
    "libs"
    "libs/glm"
    "libs/termcolor"
)

target_link_libraries(game_engine PRIVATE SDL3_ttf::SDL3_ttf SDL3::SDL3 SDL3_image::SDL3_image)

# Include tests
add_subdirectory(tests)
