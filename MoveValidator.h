#pragma once
#include <vector>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include "Board.h"

// Stateless move validation logic.
// All functions take a const Board reference — nothing is mutated here.
namespace MoveValidator {

    // Represents one legal jump: which peg moves, which it jumps, where it lands.
    struct Move {
        sf::Vector2i from;
        sf::Vector2i over;
        sf::Vector2i to;
    };

    // AC 4.1: returns all valid jumps available from a specific cell.
    // Empty if the cell has no peg or no legal jumps exist.
    std::vector<Move> getValidMoves(const Board& board, sf::Vector2i pos);

    // AC 4.7 / 5.2: returns true if any peg on the board has at least one valid jump.
    bool hasAnyMoves(const Board& board);

    // AC 6.2: collects all valid moves board-wide and returns one chosen at random.
    // Returns an empty optional if no moves exist (AC 6.6).
    std::optional<Move> pickRandomMove(const Board& board);

    // AC 4.3 / 4.4: checks whether a specific from->to jump is legal.
    // Derives the jumped cell automatically from the midpoint.
    bool isValidMove(const Board& board, sf::Vector2i from, sf::Vector2i to);

} // namespace MoveValidator