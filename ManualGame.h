#pragma once
#include "Game.h"

// -----------------------------------------------------------------------
// ManualGame — human plays every move (US4, US5, US8)
// Adds the Randomize button (AC 8.3, 8.4).
// onMoveCompleted checks win/loss only — no computer response.
// -----------------------------------------------------------------------
class ManualGame : public Game {
public:
    ManualGame();

protected:
    GameMode currentMode() const override { return GameMode::Manual; }
    void onMoveCompleted()                   override;
    void onNewGameStarted()                  override {}
    void renderExtras(sf::RenderWindow& win) override;
    bool handleExtraClick(sf::Vector2f pos)  override;

private:
    sf::RectangleShape randButton;
    sf::Text           randButtonText;
    bool randButtonContains(sf::Vector2f pos) const;
};