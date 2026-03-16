#include <gtest/gtest.h>
#include "Board.h"

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
// AC 2.2, 2.3 - All three board types produce valid cells
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