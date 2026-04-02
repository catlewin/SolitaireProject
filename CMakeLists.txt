cmake_minimum_required(VERSION 3.10)
project(SolitaireProject)

set(CMAKE_CXX_STANDARD 17)

find_library(SFML_SYSTEM   sfml-system   REQUIRED)
find_library(SFML_WINDOW   sfml-window   REQUIRED)
find_library(SFML_GRAPHICS sfml-graphics REQUIRED)

include_directories(
        /Library/Frameworks/sfml-system.framework/Headers
        /Library/Frameworks/sfml-window.framework/Headers
        /Library/Frameworks/sfml-graphics.framework/Headers
)

# --- Sources shared between main app and tests ---
set(GAME_SOURCES
        # Core board hierarchy
        Board.cpp
        EnglishBoard.cpp
        HexagonBoard.cpp
        DiamondBoard.cpp
        # Move logic
        MoveValidator.cpp
        # Game state
        GameState.cpp
        # UI popups (Popup base + subclasses)
        Popup.cpp
        SetupPopup.cpp
        GameOverUI.cpp
        # Game hierarchy
        Game.cpp
        ManualGame.cpp
        AutomatedGame.cpp
        GameFactory.cpp
)

# --- Main game executable ---
add_executable(solitaire
        main.cpp
        ${GAME_SOURCES}
)

target_link_libraries(solitaire
        ${SFML_GRAPHICS} ${SFML_WINDOW} ${SFML_SYSTEM}
)

target_include_directories(solitaire PRIVATE ${CMAKE_SOURCE_DIR})

set_target_properties(solitaire PROPERTIES
        BUILD_RPATH  "/Library/Frameworks"
        INSTALL_RPATH "/Library/Frameworks"
)

# --- Google Test ---
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

# --- Test executable ---
add_executable(solitaire_tests
        tests/BoardConfigTests.cpp
        tests/MoveValidatorTests.cpp
        tests/GameStateTests.cpp
        tests/BoardTests.cpp
        ${GAME_SOURCES}
)

target_link_libraries(solitaire_tests
        gtest_main
        ${SFML_GRAPHICS} ${SFML_WINDOW} ${SFML_SYSTEM}
)

target_include_directories(solitaire_tests PRIVATE ${CMAKE_SOURCE_DIR})

set_target_properties(solitaire_tests PROPERTIES
        BUILD_RPATH  "/Library/Frameworks"
        INSTALL_RPATH "/Library/Frameworks"
)

include(GoogleTest)
gtest_discover_tests(solitaire_tests)