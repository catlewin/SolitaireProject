#include "MoveValidator.h"

namespace MoveValidator {

// All four cardinal directions (col offset, row offset).
// Same directions apply to English, Hexagon, and Diamond —
// the board shape mask already excludes unreachable cells.
    static const sf::Vector2i DIRECTIONS[4] = {
            { 0, -2},  // up
            { 0,  2},  // down
            {-2,  0},  // left
            { 2,  0}   // right
    };

// -----------------------------------------------------------------------
// Internal helper
// -----------------------------------------------------------------------

// Returns true if jumping from `from` in direction `dir` is a legal move:
//   - the cell being jumped over contains a peg
//   - the landing cell is empty and playable
    static bool checkDirection(const Board& board,
                               sf::Vector2i from,
                               sf::Vector2i dir)
    {
        sf::Vector2i over = { from.x + dir.x / 2, from.y + dir.y / 2 };
        sf::Vector2i to   = { from.x + dir.x,     from.y + dir.y     };

        const Cell* overCell = board.getCell(over.x, over.y);
        const Cell* toCell   = board.getCell(to.x,   to.y);

        if (!overCell || !toCell)         return false; // out of bounds
        if (!overCell->hasPeg())          return false; // AC 4.3: must jump over a peg
        if (toCell->state != CellState::Empty) return false; // AC 4.3: must land on empty
        if (!toCell->isPlayable())        return false; // must be a valid cell

        return true;
    }

// -----------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------

// AC 4.1: all valid jumps from a given position
    std::vector<Move> getValidMoves(const Board& board, sf::Vector2i pos) {
        std::vector<Move> moves;

        const Cell* cell = board.getCell(pos.x, pos.y);
        if (!cell || !cell->hasPeg()) return moves; // AC 4.2: no peg = no moves

        for (const auto& dir : DIRECTIONS) {
            if (checkDirection(board, pos, dir)) {
                Move m;
                m.from = pos;
                m.over = { pos.x + dir.x / 2, pos.y + dir.y / 2 };
                m.to   = { pos.x + dir.x,     pos.y + dir.y     };
                moves.push_back(m);
            }
        }

        return moves;
    }

// AC 4.7 / 5.2: scan every cell on the board for any available jump
    bool hasAnyMoves(const Board& board) {
        for (int row = 0; row < board.getSize(); ++row) {
            for (int col = 0; col < board.getSize(); ++col) {
                const Cell* cell = board.getCell(col, row);
                if (!cell || !cell->hasPeg()) continue;

                if (!getValidMoves(board, {col, row}).empty()) {
                    return true;
                }
            }
        }
        return false;
    }

// AC 4.3 / 4.4: validate a specific from->to jump
// Derives the jumped cell from the midpoint — caller doesn't need to compute it.
    bool isValidMove(const Board& board, sf::Vector2i from, sf::Vector2i to) {
        sf::Vector2i delta = { to.x - from.x, to.y - from.y };

        // Must be exactly 2 steps in one cardinal direction
        bool horizontal = (delta.y == 0 && (delta.x == 2 || delta.x == -2));
        bool vertical   = (delta.x == 0 && (delta.y == 2 || delta.y == -2));
        if (!horizontal && !vertical) return false; // AC 4.4: not a valid jump distance

        sf::Vector2i over = { from.x + delta.x / 2, from.y + delta.y / 2 };

        const Cell* fromCell = board.getCell(from.x, from.y);
        const Cell* overCell = board.getCell(over.x, over.y);
        const Cell* toCell   = board.getCell(to.x,   to.y);

        if (!fromCell || !overCell || !toCell) return false;
        if (!fromCell->hasPeg())               return false; // source must have a peg
        if (!overCell->hasPeg())               return false; // must jump over a peg
        if (toCell->state != CellState::Empty) return false; // destination must be empty
        if (!toCell->isPlayable())             return false; // must be on the board

        return true;
    }

} // namespace MoveValidator