#include <gtest/gtest.h>
#include "GameState.h"
#include "Board.h"

static Board makeFreshBoard(int size = 7) {
    Board board;
    BoardConfig config{ size, BoardType::English };
    board.init(config);
    return board;
}

// -----------------------------------------------------------------------
// AC 3.1, 3.2, 3.4 - startGame initialises clean state
// -----------------------------------------------------------------------
TEST(GameStateTest, StartGameSetsInitialPegCount) {
Board board = makeFreshBoard();
GameState state;
state.startGame(board.getPegCount());
EXPECT_EQ(state.pegCount, board.getPegCount());
}

TEST(GameStateTest, StartGameClearsGameOverFlag) {
GameState state;
state.gameOver = true;
state.won      = true;
Board board = makeFreshBoard();
state.startGame(board.getPegCount());
EXPECT_FALSE(state.gameOver);
EXPECT_FALSE(state.won);
}

TEST(GameStateTest, StartGameClearsSelection) {
GameState state;
state.selectPeg({ 3, 3 });
Board board = makeFreshBoard();
state.startGame(board.getPegCount());
EXPECT_FALSE(state.hasSelected);
}

// -----------------------------------------------------------------------
// AC 4.1 - selectPeg records selected position
// -----------------------------------------------------------------------
TEST(GameStateTest, SelectPegSetsHasSelected) {
GameState state;
state.startGame(10);
state.selectPeg({ 2, 3 });
EXPECT_TRUE(state.hasSelected);
EXPECT_EQ(state.selectedPos, sf::Vector2i(2, 3));
}

// -----------------------------------------------------------------------
// AC 4.5 - clearSelection resets selection state
// -----------------------------------------------------------------------
TEST(GameStateTest, ClearSelectionResetsFlags) {
GameState state;
state.startGame(10);
state.selectPeg({ 2, 3 });
state.clearSelection();
EXPECT_FALSE(state.hasSelected);
EXPECT_EQ(state.selectedPos, sf::Vector2i(-1, -1));
}

// -----------------------------------------------------------------------
// AC 4.6 - recordMove syncs peg count and clears selection
// -----------------------------------------------------------------------
TEST(GameStateTest, RecordMoveSyncsPegCount) {
Board board = makeFreshBoard();
GameState state;
state.startGame(board.getPegCount());

int centre = 3;
board.applyMove({ centre, centre + 2 },
{ centre, centre + 1 },
{ centre, centre });

state.selectPeg({ centre, centre + 2 });
state.recordMove(board);

EXPECT_EQ(state.pegCount, board.getPegCount());
}

TEST(GameStateTest, RecordMoveClearsSelection) {
Board board = makeFreshBoard();
GameState state;
state.startGame(board.getPegCount());

int centre = 3;
state.selectPeg({ centre, centre + 2 });
board.applyMove({ centre, centre + 2 },
{ centre, centre + 1 },
{ centre, centre });
state.recordMove(board);

EXPECT_FALSE(state.hasSelected);
}

// -----------------------------------------------------------------------
// AC 5.1 - checkWin returns true when exactly one peg remains
// -----------------------------------------------------------------------
TEST(GameStateTest, CheckWinTrueWhenOnePegLeft) {
GameState state;
state.startGame(1);
EXPECT_TRUE(state.checkWin());
}

TEST(GameStateTest, CheckWinFalseWhenMultiplePegsLeft) {
GameState state;
state.startGame(5);
EXPECT_FALSE(state.checkWin());
}

// -----------------------------------------------------------------------
// AC 5.2 - checkLoss returns true when no moves remain and pegs > 1
// -----------------------------------------------------------------------
TEST(GameStateTest, CheckLossTrueWhenStuck) {
// Build a stuck board with two isolated pegs
Board board;
BoardConfig config{ 5, BoardType::English };
board.init(config);

for (int row = 0; row < 5; ++row)
for (int col = 0; col < 5; ++col) {
Cell* c = board.getCell(col, row);
if (c && c->isPlayable()) c->state = CellState::Empty;
}

Cell* a = board.getCell(1, 2);
Cell* b = board.getCell(3, 2);
if (a) a->state = CellState::Peg;
if (b) b->state = CellState::Peg;

GameState state;
state.startGame(2);
state.pegCount = 2; // sync manually since board pegs were set directly

EXPECT_TRUE(state.checkLoss(board));
}

TEST(GameStateTest, CheckLossFalseWhenMovesExist) {
Board board = makeFreshBoard();
GameState state;
state.startGame(board.getPegCount());
EXPECT_FALSE(state.checkLoss(board));
}

TEST(GameStateTest, CheckLossFalseWhenOnePegLeft) {
Board board = makeFreshBoard();
GameState state;
state.startGame(1);
state.pegCount = 1;
EXPECT_FALSE(state.checkLoss(board)); // win condition, not loss
}

// -----------------------------------------------------------------------
// AC 5.1, 5.2 - recordMove sets gameOver and won correctly
// -----------------------------------------------------------------------
TEST(GameStateTest, RecordMoveSetsWonWhenOnePegLeft) {
Board board;
BoardConfig config{ 5, BoardType::English };
board.init(config);

// Clear all but two adjacent pegs so one move ends the game
for (int row = 0; row < 5; ++row)
for (int col = 0; col < 5; ++col) {
Cell* c = board.getCell(col, row);
if (c && c->isPlayable()) c->state = CellState::Empty;
}

// Two pegs with an empty landing cell: (2,2) empty, (2,3) peg, (2,4) peg
board.getCell(2, 3)->state = CellState::Peg;
board.getCell(2, 4)->state = CellState::Peg;

GameState state;
state.startGame(2);
state.pegCount = 2;

board.applyMove({ 2, 4 }, { 2, 3 }, { 2, 2 });
state.recordMove(board);

EXPECT_TRUE(state.gameOver);
EXPECT_TRUE(state.won);
}

// -----------------------------------------------------------------------
// AC 5.3 - pegCount reflects final count after game over
// -----------------------------------------------------------------------
TEST(GameStateTest, PegCountCorrectAfterGameOver) {
GameState state;
state.startGame(10);
state.pegCount = 1; // simulate win
EXPECT_EQ(state.pegCount, 1);
EXPECT_TRUE(state.checkWin());
}

// -----------------------------------------------------------------------
// AC 5.5 - gameOver flag prevents further interaction (checked by Game controller)
// -----------------------------------------------------------------------
TEST(GameStateTest, GameOverFlagSetAfterWin) {
Board board;
BoardConfig config{ 5, BoardType::English };
board.init(config);

for (int row = 0; row < 5; ++row)
for (int col = 0; col < 5; ++col) {
Cell* c = board.getCell(col, row);
if (c && c->isPlayable()) c->state = CellState::Empty;
}

board.getCell(2, 3)->state = CellState::Peg;
board.getCell(2, 4)->state = CellState::Peg;

GameState state;
state.startGame(2);
state.pegCount = 2;

board.applyMove({ 2, 4 }, { 2, 3 }, { 2, 2 });
state.recordMove(board);

EXPECT_TRUE(state.gameOver);
}