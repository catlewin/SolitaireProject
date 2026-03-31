#pragma once
#include "Game.h"

// -----------------------------------------------------------------------
// AutomatedGame — human makes one move, computer responds (US6, US7)
// Adds the timer-driven computer turn.
// No randomizer button (AC 8.3).
// -----------------------------------------------------------------------
class AutomatedGame : public Game {
public:
    AutomatedGame();

protected:
    GameMode currentMode() const override { return GameMode::Automated; }
    void onMoveCompleted()  override;  // schedules computer move
    void onNewGameStarted() override { computerTurn = false; }
    void update()           override;  // drives the computer-turn timer

    // Block all board clicks while computer is thinking (AC 6.2)
    bool handleExtraClick(sf::Vector2f) override { return computerTurn; }

private:
    bool                 computerTurn = false;
    sf::Clock            computerClock;
    MoveValidator::Move  pendingMove;
    static constexpr float COMPUTER_DELAY = 0.6f;

    void triggerComputerMove();
    void applyComputerMove();
};