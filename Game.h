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
    // Randomizer button (AC 8.3, 8.4 — manual mode only)
    // -----------------------------------------------------------------------
    sf::RectangleShape randButton;
    sf::Text           randButtonText;
    bool randButtonContains(sf::Vector2f pos) const;

    // -----------------------------------------------------------------------
    // Automated mode (US6)
    // -----------------------------------------------------------------------
    bool          computerTurn = false; // true while waiting to apply computer move
    sf::Clock     computerClock;        // measures delay before computer acts
    static constexpr float COMPUTER_DELAY = 0.6f; // seconds

    void triggerComputerMove();   // AC 6.2: pick and schedule a computer move
    void applyComputerMove();     // executes the scheduled move

    MoveValidator::Move pendingMove; // the move the computer will make

    // -----------------------------------------------------------------------
    // Game loop
    // -----------------------------------------------------------------------
    void processEvents();
    void update();   // AC 6.2: drives computer-turn timer
    void render();

    // -----------------------------------------------------------------------
    // Event handlers
    // -----------------------------------------------------------------------
    void handleClick(sf::Vector2f mousePos); // AC 4.1-4.5, 5.5, 6.1, 6.3

    // -----------------------------------------------------------------------
    // Game flow
    // -----------------------------------------------------------------------
    void startNewGame(const BoardConfig& config); // AC 3.1, 3.2, 3.4
    void onMoveCompleted(bool humanMove);          // AC 4.6, 5.1, 5.2, 6.2
};