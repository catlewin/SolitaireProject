#pragma once
#include <SFML/System/Vector2.hpp>
#include "Board.h"
#include "MoveValidator.h"

// Tracks all mutable game state during an active session.
// GameState does not own Board or MoveValidator — the Game controller
// passes them in when needed, keeping this class as pure state data.
class GameState {
public:
    // -----------------------------------------------------------------------
    // State data
    // -----------------------------------------------------------------------
    bool gameOver    = false; // AC 5.5 / 7.5: locks the board when true
    bool won         = false; // AC 5.1 / 7.1: true if exactly one peg remains
    bool hasSelected = false; // AC 4.1: whether a peg is currently selected

    sf::Vector2i selectedPos;  // AC 4.1: grid position of selected peg
    int pegCount = 0;          // AC 4.6, 5.3: decremented on each valid move

    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------

    // AC 3.1, 3.2, 3.4: initialise/reset state for a new game
    void startGame(int initialPegCount);

    // -----------------------------------------------------------------------
    // Selection (AC 4.1, 4.2, 4.5)
    // -----------------------------------------------------------------------
    void selectPeg(sf::Vector2i pos);
    void clearSelection();

    // -----------------------------------------------------------------------
    // Move recording (AC 4.6)
    // -----------------------------------------------------------------------

    // Call after a valid move has been applied to the board.
    // Decrements pegCount and checks win/loss conditions.
    // Requires the board so win/loss can be evaluated immediately.
    void recordMove(const Board& board);

    // -----------------------------------------------------------------------
    // Win / loss queries (AC 5.1, 5.2)
    // -----------------------------------------------------------------------
    bool checkWin()  const; // true if pegCount == 1
    bool checkLoss(const Board& board) const; // true if no moves remain and pegCount > 1
};