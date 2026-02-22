cmake_minimum_required(VERSION 3.16)
project(GameProject CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

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

# Перед сборкой для itch нужно не забыть выключить
option(ENABLE_CHEATS "Enable cheats menu with ImGui" ON)

if(ENABLE_CHEATS)
    set(IMGUI_SOURCES
        libs/imgui/imgui.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/imgui_impl_sdl3.cpp
        libs/imgui/imgui_impl_sdlrenderer3.cpp
    )
else()
    set(IMGUI_SOURCES)
endif()

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(game_engine ${SOURCES} ${IMGUI_SOURCES})

target_include_directories(game_engine PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
    "libs"
    "libs/glm"
    "libs/termcolor"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/base64/include"
)
if(ENABLE_CHEATS)
    target_include_directories(game_engine PRIVATE "libs/imgui")
    target_compile_definitions(game_engine PRIVATE ENABLE_CHEATS=1)
endif()

target_link_libraries(game_engine PRIVATE SDL3_ttf::SDL3_ttf SDL3::SDL3 SDL3_image::SDL3_image)

if(WIN32)
    set_target_properties(game_engine PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()

target_compile_options(game_engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
)

# Include tests
add_subdirectory(tests)
