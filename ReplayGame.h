#pragma once
#include "Game.h"
#include "GameReplayer.h"

// -----------------------------------------------------------------------
// ReplayGame — read-only game driven by a GameReplayer
//
// AC 2.3: events applied automatically on a timer
// AC 2.4: all board clicks are blocked
// AC 2.6: GameOverUI shown after last event
// AC 6.2: same delay constant as AutomatedGame
// AC 6.3: status label shows Human / Computer and move counter
// AC 7.5: move counter shown in place of the Randomize button
// AC 7.8: no Record toggle shown during replay
// -----------------------------------------------------------------------
class ReplayGame : public Game {
public:
    explicit ReplayGame(GameReplayer replayer);

protected:
    GameMode currentMode()    const override;
    void onMoveCompleted()          override {}   // unused — replay drives moves
    void onNewGameStarted()         override {}
    void update()                   override;
    void renderExtras(sf::RenderWindow& win) override;

    // AC 2.4: block all board interaction
    bool handleExtraClick(sf::Vector2f) override { return true; }

private:
    GameReplayer      replayer;
    sf::Clock         replayClock;
    bool              waitingForNext = true;
    sf::Text          statusText;

    // AC 6.3: visual indicator for last move owner
    bool             hasLastMove   = false;
    sf::Vector2i     lastMoveDest  = { -1, -1 };
    MoveOwner        lastMoveOwner = MoveOwner::Human;

    static constexpr float REPLAY_DELAY = 0.7f;

    void applyNextEvent();
    void applyMoveEvent(const MoveEvent& me);
    void applyRandomizeEvent(const RandomizeEvent& re);
    void updateStatusText(const std::string& label);
    void drawLastMoveIndicator(sf::RenderWindow& win) const;
};