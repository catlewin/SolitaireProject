#include <gtest/gtest.h>
#include "MoveValidator.h"
#include "Board.h"

// Helper: set up a board and return a reference to it
static Board makeBoard(int size = 7, BoardType type = BoardType::English) {
    Board board;
    BoardConfig config{ size, type };
    board.init(config);
    return board;
}

// -----------------------------------------------------------------------
// AC 4.1 - Valid moves returned for a peg with available jumps
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, ReturnsValidMovesForPegWithJumps) {
Board board = makeBoard();
int centre = 7 / 2; // 3

// The peg two rows below centre can jump over the peg at centre+1 into centre
sf::Vector2i pos = { centre, centre + 2 };
auto moves = MoveValidator::getValidMoves(board, pos);
EXPECT_FALSE(moves.empty());
}

TEST(MoveValidatorTest, MoveHasCorrectFromOverTo) {
Board board = makeBoard();
int centre = 3;

sf::Vector2i from = { centre, centre + 2 };
auto moves = MoveValidator::getValidMoves(board, from);

// Find the upward move
bool found = false;
for (const auto& m : moves) {
if (m.to == sf::Vector2i{ centre, centre }) {
EXPECT_EQ(m.from, from);
EXPECT_EQ(m.over, sf::Vector2i(centre, centre + 1));
found = true;
}
}
EXPECT_TRUE(found) << "Expected upward jump not found";
}

// -----------------------------------------------------------------------
// AC 4.2 - No moves returned for a peg with no valid jumps
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, ReturnsEmptyForEmptyCell) {
Board board = makeBoard();
int centre = 3;
// Centre is empty on init
auto moves = MoveValidator::getValidMoves(board, { centre, centre });
EXPECT_TRUE(moves.empty());
}

TEST(MoveValidatorTest, ReturnsEmptyForInvalidCell) {
Board board = makeBoard();
// (0,0) is an invalid corner cell on English board
auto moves = MoveValidator::getValidMoves(board, { 0, 0 });
EXPECT_TRUE(moves.empty());
}

// -----------------------------------------------------------------------
// AC 4.3 - isValidMove returns true for a legal jump
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, IsValidMoveReturnsTrueForLegalJump) {
Board board = makeBoard();
int centre = 3;
// From (centre, centre+2) over (centre, centre+1) to (centre, centre)
EXPECT_TRUE(MoveValidator::isValidMove(board,
                                       { centre, centre + 2 },
                                       { centre, centre }));
}

// -----------------------------------------------------------------------
// AC 4.4 - isValidMove returns false for illegal jumps
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, IsValidMoveReturnsFalseForDiagonalJump) {
Board board = makeBoard();
int centre = 3;
EXPECT_FALSE(MoveValidator::isValidMove(board,
                                        { centre, centre + 2 },
                                        { centre + 2, centre }));
}

TEST(MoveValidatorTest, IsValidMoveReturnsFalseForOneStepMove) {
Board board = makeBoard();
int centre = 3;
EXPECT_FALSE(MoveValidator::isValidMove(board,
                                        { centre, centre + 2 },
                                        { centre, centre + 1 }));
}

TEST(MoveValidatorTest, IsValidMoveReturnsFalseWhenNoJumpablePeg) {
Board board = makeBoard();
int centre = 3;
// (centre, centre) is empty — nothing to jump over from (centre, centre-2)
// since (centre, centre-1) has a peg but destination (centre, centre) is empty...
// actually valid — test jumping over empty cell instead
// Place scenario: remove the peg at (centre, centre+1) so there's nothing to jump
board.applyMove({ centre, centre + 2 },
{ centre, centre + 1 },
{ centre, centre });
// Now (centre, centre+1) is empty — can't jump from (centre, centre+3)
EXPECT_FALSE(MoveValidator::isValidMove(board,
                                        { centre, centre + 3 },
                                        { centre, centre + 1 }));
}

TEST(MoveValidatorTest, IsValidMoveReturnsFalseWhenDestinationOccupied) {
Board board = makeBoard();
int centre = 3;
// (centre, centre+2) has a peg — jumping into it should fail
EXPECT_FALSE(MoveValidator::isValidMove(board,
                                        { centre, centre + 4 },
                                        { centre, centre + 2 }));
}

// -----------------------------------------------------------------------
// AC 4.5 - getValidMoves returns empty after peg is deselected/removed
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, NoMovesAfterPegRemoved) {
Board board = makeBoard();
int centre = 3;

// Remove peg at (centre, centre+2) by applying a move away
board.applyMove({ centre, centre + 2 },
{ centre, centre + 1 },
{ centre, centre });

auto moves = MoveValidator::getValidMoves(board, { centre, centre + 2 });
EXPECT_TRUE(moves.empty());
}

// -----------------------------------------------------------------------
// AC 4.7 - hasAnyMoves returns false when no moves remain
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, HasAnyMovesTrueOnFreshBoard) {
Board board = makeBoard();
EXPECT_TRUE(MoveValidator::hasAnyMoves(board));
}

TEST(MoveValidatorTest, HasAnyMovesFalseWhenBoardStuck) {
// Build a minimal stuck board: only two non-adjacent pegs, no jumps possible
Board board;
BoardConfig config{ 5, BoardType::English };
board.init(config);

// Clear all pegs then manually place two isolated pegs
for (int row = 0; row < 5; ++row)
for (int col = 0; col < 5; ++col) {
Cell* c = board.getCell(col, row);
if (c && c->isPlayable()) c->state = CellState::Empty;
}

// Place two pegs with no adjacent empty cell between them
Cell* a = board.getCell(1, 2);
Cell* b = board.getCell(3, 2);
if (a) a->state = CellState::Peg;
if (b) b->state = CellState::Peg;
// (2,2) is empty — but both pegs need an empty landing cell two steps away
// (0,2) and (4,2) — check those are empty (they should be)
// This means neither can jump: peg at 1 would need 3 to be empty (it's not)
// So no valid moves exist
EXPECT_FALSE(MoveValidator::hasAnyMoves(board));
}