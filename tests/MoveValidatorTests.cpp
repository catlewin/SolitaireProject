#include <gtest/gtest.h>
#include "MoveValidator.h"
#include "Board.h"
#include "EnglishBoard.h"

// Helper: construct a board via factory
static std::unique_ptr<Board> makeBoard(int size = 7,
                                        BoardType type = BoardType::English) {
    return Board::create({ size, type });
}

// Helper: set a playable cell's state directly
static void setState(Board& board, int col, int row, CellState state) {
    auto* pc = dynamic_cast<PlayableCell*>(board.getCell(col, row));
    if (pc) pc->state = state;
}

// -----------------------------------------------------------------------
// AC 4.1 - Valid moves returned for a peg with available jumps
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, ReturnsValidMovesForPegWithJumps) {
    auto board = makeBoard();
    int centre = 3;
    sf::Vector2i pos = { centre, centre + 2 };
    auto moves = MoveValidator::getValidMoves(*board, pos);
    EXPECT_FALSE(moves.empty());
}

TEST(MoveValidatorTest, MoveHasCorrectFromOverTo) {
    auto board = makeBoard();
    int centre = 3;
    sf::Vector2i from = { centre, centre + 2 };
    auto moves = MoveValidator::getValidMoves(*board, from);

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
    auto board = makeBoard();
    int centre = 3;
    auto moves = MoveValidator::getValidMoves(*board, { centre, centre });
    EXPECT_TRUE(moves.empty());
}

TEST(MoveValidatorTest, ReturnsEmptyForInvalidCell) {
    auto board = makeBoard();
    auto moves = MoveValidator::getValidMoves(*board, { 0, 0 });
    EXPECT_TRUE(moves.empty());
}

// -----------------------------------------------------------------------
// AC 4.3 - isValidMove returns true for a legal jump
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, IsValidMoveReturnsTrueForLegalJump) {
    auto board = makeBoard();
    int centre = 3;
    EXPECT_TRUE(MoveValidator::isValidMove(*board,
                                           { centre, centre + 2 },
                                           { centre, centre }));
}

// -----------------------------------------------------------------------
// AC 4.4 - isValidMove returns false for illegal jumps
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, IsValidMoveReturnsFalseForDiagonalJump) {
    auto board = makeBoard();
    int centre = 3;
    EXPECT_FALSE(MoveValidator::isValidMove(*board,
                                            { centre, centre + 2 },
                                            { centre + 2, centre }));
}

TEST(MoveValidatorTest, IsValidMoveReturnsFalseForOneStepMove) {
    auto board = makeBoard();
    int centre = 3;
    EXPECT_FALSE(MoveValidator::isValidMove(*board,
                                            { centre, centre + 2 },
                                            { centre, centre + 1 }));
}

TEST(MoveValidatorTest, IsValidMoveReturnsFalseWhenNoJumpablePeg) {
    auto board = makeBoard();
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    EXPECT_FALSE(MoveValidator::isValidMove(*board,
                                            { centre, centre + 3 },
                                            { centre, centre + 1 }));
}

TEST(MoveValidatorTest, IsValidMoveReturnsFalseWhenDestinationOccupied) {
    auto board = makeBoard();
    int centre = 3;
    EXPECT_FALSE(MoveValidator::isValidMove(*board,
                                            { centre, centre + 4 },
                                            { centre, centre + 2 }));
}

// -----------------------------------------------------------------------
// AC 4.5 - getValidMoves returns empty after peg is removed
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, NoMovesAfterPegRemoved) {
    auto board = makeBoard();
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    auto moves = MoveValidator::getValidMoves(*board, { centre, centre + 2 });
    EXPECT_TRUE(moves.empty());
}

// -----------------------------------------------------------------------
// AC 4.7 - hasAnyMoves returns false when no moves remain
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, HasAnyMovesTrueOnFreshBoard) {
    auto board = makeBoard();
    EXPECT_TRUE(MoveValidator::hasAnyMoves(*board));
}

TEST(MoveValidatorTest, HasAnyMovesFalseWhenBoardStuck) {
    auto board = makeBoard(5);

    // Clear all pegs, then place two isolated pegs with no valid jumps
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 5; ++col)
            setState(*board, col, row, CellState::Empty);

    setState(*board, 1, 2, CellState::Peg);
    setState(*board, 3, 2, CellState::Peg);

    EXPECT_FALSE(MoveValidator::hasAnyMoves(*board));
}

// -----------------------------------------------------------------------
// AC 6.2 - pickRandomMove returns a valid move on a normal board
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, PickRandomMoveReturnsMovOnFreshBoard) {
    auto board = makeBoard();
    auto move = MoveValidator::pickRandomMove(*board);
    ASSERT_TRUE(move.has_value()) << "Expected a move on a fresh board";
    EXPECT_TRUE(MoveValidator::isValidMove(*board, move->from, move->to));
}

TEST(MoveValidatorTest, PickRandomMoveResultAppliedReducesPegCount) {
    auto board = makeBoard();
    int before = board->getPegCount();
    auto move = MoveValidator::pickRandomMove(*board);
    ASSERT_TRUE(move.has_value());
    board->applyMove(move->from, move->over, move->to);
    EXPECT_EQ(board->getPegCount(), before - 1);
}

// -----------------------------------------------------------------------
// AC 6.2 - after human applies a move, pickRandomMove finds a computer move
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, ComputerMoveAvailableAfterHumanMove) {
    auto board = makeBoard();
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    auto move = MoveValidator::pickRandomMove(*board);
    ASSERT_TRUE(move.has_value()) << "Computer should find a move after human move";
    EXPECT_TRUE(MoveValidator::isValidMove(*board, move->from, move->to));
}

TEST(MoveValidatorTest, ComputerMoveAppliedAfterHumanMoveFurtherReducesPegCount) {
    auto board = makeBoard();
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    int afterHuman = board->getPegCount();
    auto move = MoveValidator::pickRandomMove(*board);
    ASSERT_TRUE(move.has_value());
    board->applyMove(move->from, move->over, move->to);
    EXPECT_EQ(board->getPegCount(), afterHuman - 1);
}

// -----------------------------------------------------------------------
// AC 6.6 - pickRandomMove returns nullopt when no moves exist
// -----------------------------------------------------------------------
TEST(MoveValidatorTest, PickRandomMoveReturnsNulloptWhenStuck) {
    auto board = makeBoard(5);
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 5; ++col)
            setState(*board, col, row, CellState::Empty);
    setState(*board, 1, 2, CellState::Peg);
    setState(*board, 3, 2, CellState::Peg);
    auto move = MoveValidator::pickRandomMove(*board);
    EXPECT_FALSE(move.has_value()) << "Expected no move on a stuck board";
}