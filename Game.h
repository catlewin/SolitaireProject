#pragma once
#include <SFML/Graphics.hpp>
#include "Board.h"
#include "GameState.h"
#include "GameOverUI.h"
#include "SetupPopup.h"
#include "MoveValidator.h"

// Central controller — owns all subsystems and routes all SFML events.
// main.cpp creates one Game instance and calls run().
class Game {
public:
    Game();
    void run();

private:
    // -----------------------------------------------------------------------
    // Core SFML
    // -----------------------------------------------------------------------
    sf::RenderWindow window;
    sf::Font         font;

    // -----------------------------------------------------------------------
    // Subsystems (owned here, AC 3.1-3.4)
    // -----------------------------------------------------------------------
    Board       board;
    GameState   gameState;
    GameOverUI  gameOverUI;
    SetupPopup  setupPopup;

    // -----------------------------------------------------------------------
    // New Game button (always visible on main screen)
    // -----------------------------------------------------------------------
    sf::RectangleShape newGameButton;
    sf::Text           newGameButtonText;
    bool newGameButtonContains(sf::Vector2f pos) const;

    // -----------------------------------------------------------------------
    // Game loop
    // -----------------------------------------------------------------------
    void processEvents();
    void render();

    // -----------------------------------------------------------------------
    // Event handlers
    // -----------------------------------------------------------------------
    void handleClick(sf::Vector2f mousePos); // AC 4.1-4.5, 5.5

    // -----------------------------------------------------------------------
    // Game flow
    // -----------------------------------------------------------------------
    void startNewGame(const BoardConfig& config); // AC 3.1, 3.2, 3.4
    void onMoveCompleted();                        // AC 4.6, 5.1, 5.2
};