#include "ReplayGame.h"
#include "Board.h"
#include "Cell.h"

ReplayGame::ReplayGame(GameReplayer rep)
        : Game(), replayer(std::move(rep)), statusText(font)
{
    statusText.setCharacterSize(13);
    statusText.setFillColor(sf::Color(80, 80, 80));
    statusText.setPosition({ 20.f, 16.f });

    // Start the game immediately using the config from the file
    BoardConfig cfg = replayer.getConfig();
    cfg.replay = false;   // clear flag so startNewGame doesn't re-route
    board = Board::create(cfg);
    gameState.startGame(board->getPegCount());

    updateStatusText("Starting replay…");
    replayClock.restart();
    waitingForNext = true;
}

GameMode ReplayGame::currentMode() const {
    // Always return Replay — this ensures any request for a live game
    // (Manual or Automated) triggers a ModeSwitch in startNewGame,
    // so ReplayGame never tries to run a live session itself.
    return GameMode::Replay;
}

// -----------------------------------------------------------------------
// Update — fires one event per REPLAY_DELAY seconds
// -----------------------------------------------------------------------
void ReplayGame::update() {
    if (gameState.isGameOver()) return;
    if (!waitingForNext)        return;
    if (replayClock.getElapsedTime().asSeconds() < REPLAY_DELAY) return;

    if (!replayer.hasNext()) {
        // AC 2.6: no more events — end the game
        gameState.recordMove(*board);
        onGameOver();
        waitingForNext = false;
        return;
    }

    applyNextEvent();
    replayClock.restart();
}

// -----------------------------------------------------------------------
// Apply the next event from the replayer
// -----------------------------------------------------------------------
void ReplayGame::applyNextEvent() {
    ReplayEvent ev = replayer.next();

    if (std::holds_alternative<MoveEvent>(ev)) {
        applyMoveEvent(std::get<MoveEvent>(ev));
    } else {
        applyRandomizeEvent(std::get<RandomizeEvent>(ev));
    }
}

void ReplayGame::applyMoveEvent(const MoveEvent& me) {
    board->applyMove(me.move.from, me.move.over, me.move.to);
    gameState.recordMove(*board);

    // AC 6.3: track destination for visual ring indicator
    lastMoveDest  = me.move.to;
    lastMoveOwner = me.owner;
    hasLastMove   = true;

    std::string owner = (me.owner == MoveOwner::Human) ? "Human" : "Computer";
    std::string label = owner + " — Move "
                        + std::to_string(replayer.currentIndex())
                        + " of "
                        + std::to_string(replayer.totalEvents());
    updateStatusText(label);

    // Tint status text to match ring colour
    statusText.setFillColor(me.owner == MoveOwner::Human
                            ? sf::Color(200, 100, 0)     // orange
                            : sf::Color(130, 40, 190));  // purple

    if (gameState.isGameOver()) {
        onGameOver();
        waitingForNext = false;
    }
}

// AC 4.2: restore board directly from snapshot — never calls randomizeBoard()
void ReplayGame::applyRandomizeEvent(const RandomizeEvent& re) {
    int idx = 0;
    int size = board->getSize();
    for (int row = 0; row < size && idx < (int)re.snapshot.size(); ++row) {
        for (int col = 0; col < size && idx < (int)re.snapshot.size(); ++col) {
            auto* pc = dynamic_cast<PlayableCell*>(board->getCell(col, row));
            if (!pc) continue;
            pc->state = (re.snapshot[idx] == 'P') ? CellState::Peg : CellState::Empty;
            ++idx;
        }
    }

    std::string label = "Randomize — Move "
                        + std::to_string(replayer.currentIndex())
                        + " of "
                        + std::to_string(replayer.totalEvents());
    updateStatusText(label);
    statusText.setFillColor(sf::Color(80, 80, 80));  // neutral grey for randomize
    hasLastMove = false;  // clear ring during randomize event

    // AC 4.3: snapshot is applied immediately — no extra delay
    // The clock will be restarted by update() after this returns,
    // so the next move waits a full REPLAY_DELAY.
}

// -----------------------------------------------------------------------
// Render status label and last-move indicator ring (AC 6.3, 7.5)
// -----------------------------------------------------------------------
void ReplayGame::renderExtras(sf::RenderWindow& win) {
    win.draw(statusText);
    if (hasLastMove) drawLastMoveIndicator(win);
}

// Draw a coloured ring around the destination peg of the last move.
// Orange for human, purple for computer — distinct from all existing peg colours.
void ReplayGame::drawLastMoveIndicator(sf::RenderWindow& win) const {
    const Cell* cell = board->getCell(lastMoveDest.x, lastMoveDest.y);
    if (!cell) return;

    float spacing = board->getCellSpacing();
    float radius  = spacing * 0.35f;

    // Outer ring — slightly larger than the peg
    sf::CircleShape ring(radius + 5.f);
    ring.setOrigin({ radius + 5.f, radius + 5.f });
    ring.setPosition(cell->screenPos);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(3.f);

    if (lastMoveOwner == MoveOwner::Human) {
        // Orange — warm, distinct from blue pegs
        ring.setOutlineColor(sf::Color(255, 140, 0));
    } else {
        // Purple — cool, clearly different from orange and blue
        ring.setOutlineColor(sf::Color(160, 60, 220));
    }

    win.draw(ring);
}

void ReplayGame::updateStatusText(const std::string& label) {
    statusText.setString(label);
}