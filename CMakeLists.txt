cmake_minimum_required(VERSION 3.10)
project(SolitaireProject)

set(CMAKE_CXX_STANDARD 17)

# Find SFML frameworks
find_library(SFML_SYSTEM sfml-system REQUIRED)
find_library(SFML_WINDOW sfml-window REQUIRED)
find_library(SFML_GRAPHICS sfml-graphics REQUIRED)

# Set include directories - point to Headers subdirectory
include_directories(
        /Library/Frameworks/sfml-system.framework/Headers
        /Library/Frameworks/sfml-window.framework/Headers
        /Library/Frameworks/sfml-graphics.framework/Headers
)

# GUI Demo executable
add_executable(practice_gui practice_gui.cpp)

target_link_libraries(practice_gui
        ${SFML_GRAPHICS}
        ${SFML_WINDOW}
        ${SFML_SYSTEM}
)

set_target_properties(practice_gui PROPERTIES
        BUILD_RPATH "/Library/Frameworks"
        INSTALL_RPATH "/Library/Frameworks"
)

# Google Test setup
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(
        solitaire_tests
        tests/test_main.cpp
)

target_link_libraries(
        solitaire_tests
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(solitaire_tests)