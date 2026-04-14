#pragma once
#include "Game.h"
#include "GameRecorder.h"

// -----------------------------------------------------------------------
// ManualGame — human plays every move (US4, US5, US8)
// Adds the Randomize button (AC 8.3, 8.4).
// Sprint 4: optionally records moves and randomize events (US1, US3).
// -----------------------------------------------------------------------
class ManualGame : public Game {
public:
    ManualGame();

protected:
    GameMode currentMode() const override { return GameMode::Manual; }
    void onMoveCompleted()                   override;
    void onNewGameStarted()                  override;
    void setPendingRecord(bool r)            override { pendingRecord = r; }
    void renderExtras(sf::RenderWindow& win) override;
    bool handleExtraClick(sf::Vector2f pos)  override;

private:
    sf::RectangleShape randButton;
    sf::Text           randButtonText;
    bool randButtonContains(sf::Vector2f pos) const;

    // Sprint 4 recording
    GameRecorder recorder;
    bool         pendingRecord = false;  // set by startNewGame before onNewGameStarted()
    sf::Text     recIndicator;   // AC 7.7: "● REC" label during recording
};