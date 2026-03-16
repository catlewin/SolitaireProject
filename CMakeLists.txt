cmake_minimum_required(VERSION 3.10)
project(SolitaireProject)

set(CMAKE_CXX_STANDARD 17)

# Find SFML frameworks
find_library(SFML_SYSTEM sfml-system REQUIRED)
find_library(SFML_WINDOW sfml-window REQUIRED)
find_library(SFML_GRAPHICS sfml-graphics REQUIRED)

include_directories(
        /Library/Frameworks/sfml-system.framework/Headers
        /Library/Frameworks/sfml-window.framework/Headers
        /Library/Frameworks/sfml-graphics.framework/Headers
)

# --- Source files shared between main app and tests ---
set(GAME_SOURCES
        Board.cpp
        MoveValidator.cpp
        GameState.cpp
        SetupPopup.cpp
        GameOverUI.cpp
        Game.cpp
)

# --- Main game executable ---
add_executable(solitaire
        main.cpp
        ${GAME_SOURCES}
)

target_link_libraries(solitaire
        ${SFML_GRAPHICS}
        ${SFML_WINDOW}
        ${SFML_SYSTEM}
)

set_target_properties(solitaire PROPERTIES
        BUILD_RPATH "/Library/Frameworks"
        INSTALL_RPATH "/Library/Frameworks"
)

# --- Google Test setup ---
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# --- Test executable ---
add_executable(solitaire_tests
        # Add future test files here:
       #  tests/BoardConfigTests.cpp
        # tests/BoardTests.cpp
        # tests/MoveValidatorTests.cpp
        # tests/GameStateTests.cpp
        ${GAME_SOURCES}
)

target_link_libraries(solitaire_tests
        gtest_main
        ${SFML_GRAPHICS}
        ${SFML_WINDOW}
        ${SFML_SYSTEM}
)

set_target_properties(solitaire_tests PROPERTIES
        BUILD_RPATH "/Library/Frameworks"
        INSTALL_RPATH "/Library/Frameworks"
)

include(GoogleTest)
gtest_discover_tests(solitaire_tests)