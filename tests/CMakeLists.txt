# Google Test configuration
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)

# Enable testing
enable_testing()

# Test executable
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Add source files needed for testing
set(TEST_ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/Entities/EntitiesManager.cpp
)

add_executable(game_engine_tests ${TEST_SOURCES} ${TEST_ENGINE_SOURCES})

target_include_directories(game_engine_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/libs/glm
    ${CMAKE_SOURCE_DIR}/libs/termcolor
)

target_link_libraries(game_engine_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
)

# Add test to CTest
add_test(NAME GameEngineTests COMMAND game_engine_tests)

