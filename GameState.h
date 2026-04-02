#pragma once
#include <SFML/System/Vector2.hpp>
#include "MoveValidator.h"

// Forward declare Board to avoid pulling in Board.h and its transitive
// dependencies (Cell.h, BoardConfig.h) into every file that uses GameState.
class Board;

// Tracks all mutable game state during an active session.
// All state is private — external code reads via getters and
// mutates only through the defined public methods.
class GameState {
public:
    // -----------------------------------------------------------------------
    // Lifecycle (AC 3.1, 3.2, 3.4)
    // -----------------------------------------------------------------------
    void startGame(int initialPegCount);

    // -----------------------------------------------------------------------
    // Selection (AC 4.1, 4.5)
    // -----------------------------------------------------------------------
    void selectPeg(sf::Vector2i pos);
    void clearSelection();

    // -----------------------------------------------------------------------
    // Move recording (AC 4.6)
    // -----------------------------------------------------------------------
    void recordMove(const Board& board);

    // -----------------------------------------------------------------------
    // Win / loss queries (AC 5.1, 5.2)
    // -----------------------------------------------------------------------
    bool checkWin()              const;
    bool checkLoss(const Board&) const;

    // -----------------------------------------------------------------------
    // Read-only accessors
    // -----------------------------------------------------------------------
    bool         isGameOver()   const { return gameOver; }
    bool         isWon()        const { return won; }
    bool         hasSelection() const { return hasSelected; }
    sf::Vector2i getSelected()  const { return selectedPos; }
    int          getPegCount()  const { return pegCount; }

    // -----------------------------------------------------------------------
    // Test support — allows unit tests to construct specific game states
    // without going through the full game loop
    // -----------------------------------------------------------------------
    void forceStateForTesting(int pegs, bool over = false, bool wonFlag = false) {
        pegCount = pegs;
        gameOver = over;
        won      = wonFlag;
    }

private:
    bool         gameOver    = false;
    bool         won         = false;
    bool         hasSelected = false;
    sf::Vector2i selectedPos = { -1, -1 };
    int          pegCount    = 0;
};