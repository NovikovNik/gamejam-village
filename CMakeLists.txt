cmake_minimum_required(VERSION 3.16)
project(GameProject C CXX)

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

if(APPLE)
    set(APP_ICON "AAAB.icns")
    set(APP_ICON_PATH "${CMAKE_SOURCE_DIR}/MacOs/${APP_ICON}")
    
    # Mark the file as a resource so it's copied to Contents/Resources
    set_source_files_properties("${APP_ICON_PATH}" PROPERTIES 
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(AAAB ${SOURCES} ${IMGUI_SOURCES} ${APP_ICON_PATH} libs/stb_vorbis/stb_vorbis.c)

target_include_directories(AAAB PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
    "libs"
    "libs/glm"
    "libs/termcolor"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/base64/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb_vorbis"
)
if(ENABLE_CHEATS)
    target_include_directories(AAAB PRIVATE "libs/imgui")
    target_compile_definitions(AAAB PRIVATE ENABLE_CHEATS=1)
endif()

target_link_libraries(AAAB PRIVATE SDL3_ttf::SDL3_ttf SDL3::SDL3 SDL3_image::SDL3_image)

if(WIN32)
    set_target_properties(AAAB PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()

if(APPLE)
    set_target_properties(AAAB PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOs/Info.plist.in"
        # IMPORTANT: This must be just the filename, e.g., "AAAB.icns"
        MACOSX_BUNDLE_ICON_FILE "${APP_ICON}"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AAAB"
    )

    # Assets logic...
    set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/Assets")
    add_custom_command(TARGET AAAB POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSETS_DIR}"
            "$<TARGET_BUNDLE_CONTENT_DIR:AAAB>/Resources/Assets"
    )
endif()

target_compile_options(AAAB PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
)

# Include tests
add_subdirectory(tests)
