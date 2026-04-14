#pragma once
#include "Game.h"
#include "GameRecorder.h"

// -----------------------------------------------------------------------
// AutomatedGame — human makes one move, computer responds (US6, US7)
// Sprint 4: optionally records both human and computer moves (US5).
// No randomizer button (AC 8.3).
// -----------------------------------------------------------------------
class AutomatedGame : public Game {
public:
    AutomatedGame();

protected:
    GameMode currentMode() const override { return GameMode::Automated; }
    void onMoveCompleted()        override;
    void onNewGameStarted()       override;
    void setPendingRecord(bool r) override { pendingRecord = r; }
    void update()                 override;
    void renderExtras(sf::RenderWindow& win) override;

    // Block all board clicks while computer is thinking (AC 6.2)
    bool handleExtraClick(sf::Vector2f) override { return computerTurn; }

private:
    bool                 computerTurn = false;
    sf::Clock            computerClock;
    MoveValidator::Move  pendingMove;
    static constexpr float COMPUTER_DELAY = 0.6f;

    GameRecorder recorder;
    bool         pendingRecord = false;  // set by startNewGame before onNewGameStarted()
    sf::Text     recIndicator;   // AC 7.7

    void triggerComputerMove();
    void applyComputerMove();
};