#include <gtest/gtest.h>
#include "Board.h"
#include "MoveValidator.h"

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

// Helper: count valid (non-invalid) cells
static int countValidCells(Board& board, int size) {
    int count = 0;
    for (int row = 0; row < size; ++row)
        for (int col = 0; col < size; ++col) {
            const Cell* c = board.getCell(col, row);
            if (c && c->isPlayable()) count++;
        }
    return count;
}

// -----------------------------------------------------------------------
// AC 3.1 - Default config (size 7, English) initialises correctly
// -----------------------------------------------------------------------
TEST(BoardTest, DefaultConfigInitialises) {
    Board board;
    BoardConfig config; // size=7, English
    board.init(config);
    EXPECT_EQ(board.getSize(), 7);
    EXPECT_EQ(board.getType(), BoardType::English);
}

// -----------------------------------------------------------------------
// AC 3.2 - Custom config is applied
// -----------------------------------------------------------------------
TEST(BoardTest, CustomSizeApplied) {
    Board board;
    BoardConfig config;
    config.size = 9;
    config.type = BoardType::Diamond;
    board.init(config);
    EXPECT_EQ(board.getSize(), 9);
    EXPECT_EQ(board.getType(), BoardType::Diamond);
}

// -----------------------------------------------------------------------
// AC 3.3 - Centre hole is empty on init, all other valid cells have pegs
// -----------------------------------------------------------------------
TEST(BoardTest, CentreIsEmptyOnInit) {
    Board board;
    BoardConfig config;
    board.init(config);
    int centre = config.size / 2;
    const Cell* centre_cell = board.getCell(centre, centre);
    ASSERT_NE(centre_cell, nullptr);
    EXPECT_EQ(centre_cell->state, CellState::Empty);
}

TEST(BoardTest, PegCountMatchesBoardGetPegCount) {
    Board board;
    BoardConfig config;
    board.init(config);
    EXPECT_EQ(board.getPegCount(), countPegs(board, config.size));
}

TEST(BoardTest, AllValidNonCentreCellsHavePegs) {
    Board board;
    BoardConfig config;
    board.init(config);
    int centre = config.size / 2;

    for (int row = 0; row < config.size; ++row) {
        for (int col = 0; col < config.size; ++col) {
            const Cell* c = board.getCell(col, row);
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
    Board board;
    BoardConfig config{ 7, BoardType::English };
    board.init(config);
    EXPECT_GT(countValidCells(board, 7), 0);
}

TEST(BoardTest, HexagonBoardHasValidCells) {
    Board board;
    BoardConfig config{ 7, BoardType::Hexagon };
    board.init(config);
    EXPECT_GT(countValidCells(board, 7), 0);
}

TEST(BoardTest, DiamondBoardHasValidCells) {
    Board board;
    BoardConfig config{ 7, BoardType::Diamond };
    board.init(config);
    EXPECT_GT(countValidCells(board, 7), 0);
}

// -----------------------------------------------------------------------
// AC 1.5, 2.5, 3.4 - Reset wipes progress and applies new config
// -----------------------------------------------------------------------
TEST(BoardTest, ResetAppliesNewConfig) {
    Board board;
    BoardConfig config;
    board.init(config);

    BoardConfig newConfig{ 9, BoardType::Hexagon };
    board.reset(newConfig);

    EXPECT_EQ(board.getSize(), 9);
    EXPECT_EQ(board.getType(), BoardType::Hexagon);
}

TEST(BoardTest, ResetRestoresPegCount) {
    Board board;
    BoardConfig config;
    board.init(config);
    int initialCount = board.getPegCount();

// Apply a move to dirty the state
    int centre = config.size / 2;
    board.applyMove({ centre, centre + 2 },
                    { centre, centre + 1 },
                    { centre, centre });
    EXPECT_LT(board.getPegCount(), initialCount);

// Reset should restore full peg count
    board.reset(config);
    EXPECT_EQ(board.getPegCount(), initialCount);
}

// -----------------------------------------------------------------------
// AC 4.6 - applyMove removes jumped peg and decrements count
// -----------------------------------------------------------------------
TEST(BoardTest, ApplyMoveDecrementsPegCount) {
    Board board;
    BoardConfig config;
    board.init(config);
    int before = board.getPegCount();

    int centre = config.size / 2;
// Jump from (centre, centre+2) over (centre, centre+1) to (centre, centre)
    board.applyMove({ centre, centre + 2 },
                    { centre, centre + 1 },
                    { centre, centre });

    EXPECT_EQ(board.getPegCount(), before - 1);
}

TEST(BoardTest, ApplyMoveUpdatesCell_States) {
    Board board;
    BoardConfig config;
    board.init(config);

    int centre = config.size / 2;
    sf::Vector2i from = { centre, centre + 2 };
    sf::Vector2i over = { centre, centre + 1 };
    sf::Vector2i to   = { centre, centre };

    board.applyMove(from, over, to);

    EXPECT_EQ(board.getCell(from.x, from.y)->state, CellState::Empty);
    EXPECT_EQ(board.getCell(over.x, over.y)->state, CellState::Empty);
    EXPECT_EQ(board.getCell(to.x,   to.y  )->state, CellState::Peg);
}

// -----------------------------------------------------------------------
// AC 4.1 - highlightMoves marks selected and destination cells
// -----------------------------------------------------------------------
TEST(BoardTest, HighlightMovesMarksSelectedAndDestinations) {
    Board board;
    BoardConfig config;
    board.init(config);

    int centre = config.size / 2;
    sf::Vector2i selected = { centre, centre + 2 };
    std::vector<sf::Vector2i> dests = { { centre, centre } };

    board.highlightMoves(selected, dests);

    EXPECT_EQ(board.getCell(selected.x, selected.y)->state, CellState::Selected);
    EXPECT_EQ(board.getCell(centre, centre)->state,          CellState::Highlighted);
}

// -----------------------------------------------------------------------
// AC 4.5 - clearSelection restores peg and empty states
// -----------------------------------------------------------------------
TEST(BoardTest, ClearSelectionRestoresStates) {
    Board board;
    BoardConfig config;
    board.init(config);

    int centre = config.size / 2;
    sf::Vector2i selected = { centre, centre + 2 };
    board.highlightMoves(selected, { { centre, centre } });
    board.clearSelection();

    EXPECT_EQ(board.getCell(selected.x, selected.y)->state, CellState::Peg);
    EXPECT_EQ(board.getCell(centre, centre)->state,          CellState::Empty);
}
// -----------------------------------------------------------------------
// AC 8.1 - randomizeBoard shuffles peg positions (board changes)
// -----------------------------------------------------------------------
TEST(BoardTest, RandomizeBoardChangesLayout) {
    Board board;
    BoardConfig config{ 7, BoardType::English };
    board.init(config);

    // A fresh board has 32 pegs in 33 playable cells — almost no room to shuffle.
    // Apply several moves to open up the board so the shuffle has room to move pegs.
    int c = config.size / 2; // 3
    board.applyMove({ c, c + 2 }, { c, c + 1 }, { c, c });         // col 3, down→up
    board.applyMove({ c + 2, c }, { c + 1, c }, { c, c - 0 });     // right→left (lands on occupied — skip, use safe move)

    // Use a sequence of safe, valid moves on the English-7 board to open ~5 holes
    board.applyMove({ c - 2, c }, { c - 1, c }, { c + 1 - 1, c }); // may be blocked; that's fine
    // Reset and use a reliable multi-move sequence instead
    board.reset(config);

    // Apply 8 moves in a known valid chain to produce a board with ~9 empty cells
    // (each move removes one peg, starting from 32 → 24 after 8 moves)
    // Use only the well-known downward/upward jumps from the centre column
    for (int i = 0; i < 8; ++i) {
        bool moved = false;
        for (int row = 0; row < 7 && !moved; ++row) {
            for (int col = 0; col < 7 && !moved; ++col) {
                auto moves = MoveValidator::getValidMoves(board, { col, row });
                if (!moves.empty()) {
                    board.applyMove(moves[0].from, moves[0].over, moves[0].to);
                    moved = true;
                }
            }
        }
    }

    // Snapshot only playable cell states
    std::vector<CellState> before;
    for (int row = 0; row < 7; ++row)
        for (int col = 0; col < 7; ++col) {
            const Cell* cell = board.getCell(col, row);
            if (cell && cell->isPlayable()) before.push_back(cell->state);
        }

    // Run randomize multiple times — at least one run must differ
    // (with ~24 pegs in 33 cells the chance of identical shuffle is negligible)
    bool changed = false;
    for (int attempt = 0; attempt < 10 && !changed; ++attempt) {
        board.randomizeBoard();
        std::vector<CellState> after;
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 7; ++col) {
                const Cell* cell = board.getCell(col, row);
                if (cell && cell->isPlayable()) after.push_back(cell->state);
            }
        if (before != after) changed = true;
    }

    EXPECT_TRUE(changed) << "randomizeBoard never changed the layout across 10 attempts";
}

// -----------------------------------------------------------------------
// AC 8.2 - randomizeBoard preserves peg count
// -----------------------------------------------------------------------
TEST(BoardTest, RandomizeBoardPreservesPegCount) {
    Board board;
    BoardConfig config{ 7, BoardType::English };
    board.init(config);

    int before = board.getPegCount();
    board.randomizeBoard();

    EXPECT_EQ(board.getPegCount(), before);
}

TEST(BoardTest, RandomizeBoardPreservesPegCountAfterMove) {
    Board board;
    BoardConfig config{ 7, BoardType::English };
    board.init(config);

    // Make a move so peg count is no longer the initial value
    int centre = config.size / 2;
    board.applyMove({ centre, centre + 2 },
                    { centre, centre + 1 },
                    { centre, centre });

    int before = board.getPegCount();
    board.randomizeBoard();

    EXPECT_EQ(board.getPegCount(), before);
}

TEST(BoardTest, RandomizeBoardKeepsPlayableCellCount) {
    Board board;
    BoardConfig config{ 7, BoardType::English };
    board.init(config);

    int validBefore = countValidCells(board, 7);
    board.randomizeBoard();

    EXPECT_EQ(countValidCells(board, 7), validBefore);
}