#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "Board.h"
#include "GameState.h"
#include "GameOverUI.h"
#include "SetupPopup.h"
#include "MoveValidator.h"

// -----------------------------------------------------------------------
// Game — abstract base class
// Owns the window, font, board, game state, and both UI overlays.
// Provides the shared game loop, event routing, rendering, and
// new-game flow. Subclasses implement mode-specific behaviour.
// Concrete subclasses: ManualGame, AutomatedGame
// -----------------------------------------------------------------------
class Game {
public:
    Game();
    virtual ~Game() = default;

    void run();

    // If the player requests a game whose mode differs from the current
    // subclass, run() exits early and sets this config for main to act on.
    // main.cpp checks this after run() returns and rebuilds if needed.
    bool        restartRequested = false;
    BoardConfig restartConfig;

    // Factory: constructs the correct Game subclass from a config
    static std::unique_ptr<Game> create(const BoardConfig& config);

protected:
    // -----------------------------------------------------------------------
    // Core SFML
    // -----------------------------------------------------------------------
    sf::RenderWindow window;
    sf::Font         font;

    // -----------------------------------------------------------------------
    // Subsystems
    // -----------------------------------------------------------------------
    std::unique_ptr<Board> board;
    GameState              gameState;
    GameOverUI             gameOverUI;
    SetupPopup             setupPopup;

    // -----------------------------------------------------------------------
    // New Game button (always visible)
    // -----------------------------------------------------------------------
    sf::RectangleShape newGameButton;
    sf::Text           newGameButtonText;
    bool newGameButtonContains(sf::Vector2f pos) const;

    // -----------------------------------------------------------------------
    // Shared game flow
    // -----------------------------------------------------------------------
    void startNewGame(const BoardConfig& config);
    void onGameOver();

    // -----------------------------------------------------------------------
    // Shared click handling (selection, deselection, move execution)
    // -----------------------------------------------------------------------
    void handleBoardClick(sf::Vector2f mousePos);

    // -----------------------------------------------------------------------
    // Game loop — subclasses override update() and can extend render()
    // -----------------------------------------------------------------------
    void processEvents();
    virtual void update() {}
    virtual void render();

    // -----------------------------------------------------------------------
    // Mode-specific hooks — pure virtual
    // -----------------------------------------------------------------------
    virtual GameMode currentMode()    const = 0; // subclass returns its mode
    virtual void onMoveCompleted()        = 0;
    virtual void onNewGameStarted()       = 0;
    virtual void renderExtras(sf::RenderWindow& win) {}
    virtual bool handleExtraClick(sf::Vector2f pos) { return false; }

private:
    static sf::Font loadFont(const std::string& path);
};