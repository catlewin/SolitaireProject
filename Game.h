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
// Concrete subclasses: ManualGame, AutomatedGame, ReplayGame
// -----------------------------------------------------------------------
class Game {
public:
    Game();
    virtual ~Game() = default;

    enum class RunResult { Closed, ModeSwitch };
    RunResult run();

    bool        restartRequested = false;
    BoardConfig restartConfig;

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
    // Last applied move — set by handleBoardClick before onMoveCompleted()
    // Subclasses (ManualGame, AutomatedGame) read this to record the move.
    // -----------------------------------------------------------------------
    MoveValidator::Move lastMove;

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
    // Game loop
    // -----------------------------------------------------------------------
    void processEvents();
    virtual void update() {}
    virtual void render();

    // -----------------------------------------------------------------------
    // Mode-specific hooks
    // -----------------------------------------------------------------------
    virtual GameMode currentMode()    const = 0;
    virtual void onMoveCompleted()        = 0;
    virtual void onNewGameStarted()       = 0;
    virtual void setPendingRecord(bool)   {}   // subclasses store the record flag
    virtual void renderExtras(sf::RenderWindow& win) {}
    virtual bool handleExtraClick(sf::Vector2f pos) { return false; }

private:
    static sf::Font loadFont(const std::string& path);
};