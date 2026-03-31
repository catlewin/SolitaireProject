#include <gtest/gtest.h>
#include "Board.h"
#include "EnglishBoard.h"
#include "HexagonBoard.h"
#include "DiamondBoard.h"
#include "MoveValidator.h"

// Helper to get PlayableCell state safely
static CellState getState(Board& board, int col, int row) {
    const auto* pc = dynamic_cast<const PlayableCell*>(board.getCell(col, row));
    EXPECT_NE(pc, nullptr) << "Cell at (" << col << "," << row << ") is not playable";
    return pc ? pc->state : CellState::Empty;
}

// Helper: count pegs on a board
static int countPegs(Board& board, int size) {
    int count = 0;
    for (int row = 0; row < size; ++row)
        for (int col = 0; col < size; ++col) {
            const Cell* c = board.getCell(col, row);
            if (c && c->hasPeg()) count++;
        }
    return count;
}

// Helper: count valid (playable) cells
static int countValidCells(Board& board, int size) {
    int count = 0;
    for (int row = 0; row < size; ++row)
        for (int col = 0; col < size; ++col) {
            const Cell* c = board.getCell(col, row);
            if (c && c->isPlayable()) count++;
        }
    return count;
}

// Helper: set a playable cell's state directly (for test setup)
static void setState(Board& board, int col, int row, CellState state) {
    auto* pc = dynamic_cast<PlayableCell*>(board.getCell(col, row));
    if (pc) pc->state = state;
}

// -----------------------------------------------------------------------
// AC 3.1 - Default config (size 7, English) initialises correctly
// -----------------------------------------------------------------------
TEST(BoardTest, DefaultConfigInitialises) {
    auto board = Board::create({ 7, BoardType::English });
    EXPECT_EQ(board->getSize(), 7);
    EXPECT_EQ(board->getType(), BoardType::English);
}

// -----------------------------------------------------------------------
// AC 3.2 - Custom config is applied
// -----------------------------------------------------------------------
TEST(BoardTest, CustomSizeApplied) {
    auto board = Board::create({ 9, BoardType::Diamond });
    EXPECT_EQ(board->getSize(), 9);
    EXPECT_EQ(board->getType(), BoardType::Diamond);
}

// -----------------------------------------------------------------------
// AC 3.3 - Centre hole is empty on init, all other valid cells have pegs
// -----------------------------------------------------------------------
TEST(BoardTest, CentreIsEmptyOnInit) {
    auto board = Board::create({ 7, BoardType::English });
    int centre = 7 / 2;
    EXPECT_EQ(getState(*board, centre, centre), CellState::Empty);
}

TEST(BoardTest, PegCountMatchesBoardGetPegCount) {
    auto board = Board::create({ 7, BoardType::English });
    EXPECT_EQ(board->getPegCount(), countPegs(*board, 7));
}

TEST(BoardTest, AllValidNonCentreCellsHavePegs) {
    auto board = Board::create({ 7, BoardType::English });
    int centre = 7 / 2;
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 7; ++col) {
            const Cell* c = board->getCell(col, row);
            if (!c || !c->isPlayable()) continue;
            if (col == centre && row == centre) continue;
            EXPECT_TRUE(c->hasPeg())
                                << "Expected peg at (" << col << "," << row << ")";
        }
    }
}

// -----------------------------------------------------------------------
// AC 1.7, 1.8 - All three board types produce valid cells
// -----------------------------------------------------------------------
TEST(BoardTest, EnglishBoardHasValidCells) {
    auto board = Board::create({ 7, BoardType::English });
    EXPECT_GT(countValidCells(*board, 7), 0);
}

TEST(BoardTest, HexagonBoardHasValidCells) {
    auto board = Board::create({ 7, BoardType::Hexagon });
    EXPECT_GT(countValidCells(*board, 7), 0);
}

TEST(BoardTest, DiamondBoardHasValidCells) {
    auto board = Board::create({ 7, BoardType::Diamond });
    EXPECT_GT(countValidCells(*board, 7), 0);
}

// -----------------------------------------------------------------------
// AC 1.5, 3.4 - Reset wipes progress and applies new config
// -----------------------------------------------------------------------
TEST(BoardTest, ResetAppliesNewConfig) {
    auto board = Board::create({ 7, BoardType::English });
    board->reset({ 9, BoardType::Hexagon });
    EXPECT_EQ(board->getSize(), 9);
    // Type is defined by the subclass — reset doesn't change subclass type
    // so type stays English; a type change requires Board::create()
}

TEST(BoardTest, ResetRestoresPegCount) {
    auto board = Board::create({ 7, BoardType::English });
    int initialCount = board->getPegCount();

    int centre = 7 / 2;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    EXPECT_LT(board->getPegCount(), initialCount);

    board->reset({ 7, BoardType::English });
    EXPECT_EQ(board->getPegCount(), initialCount);
}

// -----------------------------------------------------------------------
// AC 4.6 - applyMove removes jumped peg and decrements count
// -----------------------------------------------------------------------
TEST(BoardTest, ApplyMoveDecrementsPegCount) {
    auto board = Board::create({ 7, BoardType::English });
    int before = board->getPegCount();
    int centre = 7 / 2;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    EXPECT_EQ(board->getPegCount(), before - 1);
}

TEST(BoardTest, ApplyMoveUpdatesCell_States) {
    auto board = Board::create({ 7, BoardType::English });
    int centre = 7 / 2;
    sf::Vector2i from = { centre, centre + 2 };
    sf::Vector2i over = { centre, centre + 1 };
    sf::Vector2i to   = { centre, centre };

    board->applyMove(from, over, to);

    EXPECT_EQ(getState(*board, from.x, from.y), CellState::Empty);
    EXPECT_EQ(getState(*board, over.x, over.y), CellState::Empty);
    EXPECT_EQ(getState(*board, to.x,   to.y  ), CellState::Peg);
}

// -----------------------------------------------------------------------
// AC 4.1 - highlightMoves marks selected and destination cells
// -----------------------------------------------------------------------
TEST(BoardTest, HighlightMovesMarksSelectedAndDestinations) {
    auto board = Board::create({ 7, BoardType::English });
    int centre = 7 / 2;
    sf::Vector2i selected = { centre, centre + 2 };
    board->highlightMoves(selected, { { centre, centre } });

    EXPECT_EQ(getState(*board, selected.x, selected.y), CellState::Selected);
    EXPECT_EQ(getState(*board, centre, centre),          CellState::Highlighted);
}

// -----------------------------------------------------------------------
// AC 4.5 - clearSelection restores peg and empty states
// -----------------------------------------------------------------------
TEST(BoardTest, ClearSelectionRestoresStates) {
    auto board = Board::create({ 7, BoardType::English });
    int centre = 7 / 2;
    sf::Vector2i selected = { centre, centre + 2 };
    board->highlightMoves(selected, { { centre, centre } });
    board->clearSelection();

    EXPECT_EQ(getState(*board, selected.x, selected.y), CellState::Peg);
    EXPECT_EQ(getState(*board, centre, centre),          CellState::Empty);
}

// -----------------------------------------------------------------------
// AC 8.1 - randomizeBoard shuffles peg positions (board changes)
// -----------------------------------------------------------------------
TEST(BoardTest, RandomizeBoardChangesLayout) {
    auto board = Board::create({ 7, BoardType::English });

    // Apply 8 moves to open up the board so the shuffle has room to work
    for (int i = 0; i < 8; ++i) {
        bool moved = false;
        for (int row = 0; row < 7 && !moved; ++row) {
            for (int col = 0; col < 7 && !moved; ++col) {
                auto moves = MoveValidator::getValidMoves(*board, { col, row });
                if (!moves.empty()) {
                    board->applyMove(moves[0].from, moves[0].over, moves[0].to);
                    moved = true;
                }
            }
        }
    }

    // Snapshot playable cell states before randomize
    std::vector<CellState> before;
    for (int row = 0; row < 7; ++row)
        for (int col = 0; col < 7; ++col) {
            const auto* pc = dynamic_cast<const PlayableCell*>(board->getCell(col, row));
            if (pc) before.push_back(pc->state);
        }

    // Retry up to 10 times — at least one shuffle must differ
    bool changed = false;
    for (int attempt = 0; attempt < 10 && !changed; ++attempt) {
        board->randomizeBoard();
        std::vector<CellState> after;
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 7; ++col) {
                const auto* pc = dynamic_cast<const PlayableCell*>(board->getCell(col, row));
                if (pc) after.push_back(pc->state);
            }
        if (before != after) changed = true;
    }
    EXPECT_TRUE(changed) << "randomizeBoard never changed the layout across 10 attempts";
}

// -----------------------------------------------------------------------
// AC 8.2 - randomizeBoard preserves peg count
// -----------------------------------------------------------------------
TEST(BoardTest, RandomizeBoardPreservesPegCount) {
    auto board = Board::create({ 7, BoardType::English });
    int before = board->getPegCount();
    board->randomizeBoard();
    EXPECT_EQ(board->getPegCount(), before);
}

TEST(BoardTest, RandomizeBoardPreservesPegCountAfterMove) {
    auto board = Board::create({ 7, BoardType::English });
    int centre = 7 / 2;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    int before = board->getPegCount();
    board->randomizeBoard();
    EXPECT_EQ(board->getPegCount(), before);
}

TEST(BoardTest, RandomizeBoardKeepsPlayableCellCount) {
    auto board = Board::create({ 7, BoardType::English });
    int validBefore = countValidCells(*board, 7);
    board->randomizeBoard();
    EXPECT_EQ(countValidCells(*board, 7), validBefore);
}