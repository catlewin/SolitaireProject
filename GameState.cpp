#include "GameState.h"

// -----------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------

// AC 3.1, 3.2, 3.4: reset all flags and counters for a fresh game
void GameState::startGame(int initialPegCount, GameMode mode) {
    pegCount    = initialPegCount;
    gameMode    = mode;
    gameOver    = false;
    won         = false;
    hasSelected = false;
    selectedPos = { -1, -1 };
}

// -----------------------------------------------------------------------
// Selection
// -----------------------------------------------------------------------

// AC 4.1: record which peg the player has chosen
void GameState::selectPeg(sf::Vector2i pos) {
    hasSelected = true;
    selectedPos = pos;
}

// AC 4.5: deselect — board highlight clearing is handled by Game controller
void GameState::clearSelection() {
    hasSelected = false;
    selectedPos = { -1, -1 };
}

// -----------------------------------------------------------------------
// Move recording
// -----------------------------------------------------------------------

// AC 4.6: called by Game controller after board.applyMove()
// Decrements peg count then evaluates win/loss immediately so
// gameOver and won are always up to date after every move.
void GameState::recordMove(const Board& board) {
    // pegCount is already decremented by Board::applyMove,
    // so sync from the board as the single source of truth.
    pegCount = board.getPegCount();
    clearSelection();

    // AC 5.1: win condition
    if (checkWin()) {
        gameOver = true;
        won      = true;
        return;
    }

    // AC 5.2: loss condition — no moves left and more than one peg remains
    if (checkLoss(board)) {
        gameOver = true;
        won      = false;
    }
}

// -----------------------------------------------------------------------
// Win / loss queries
// -----------------------------------------------------------------------

// AC 5.1: exactly one peg left = win
bool GameState::checkWin() const {
    return pegCount == 1;
}

// AC 5.2: more than one peg and no valid moves remain = loss
bool GameState::checkLoss(const Board& board) const {
    return pegCount > 1 && !MoveValidator::hasAnyMoves(board);
}