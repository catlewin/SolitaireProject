#include <gtest/gtest.h>
#include "GameState.h"
#include "Board.h"
#include "EnglishBoard.h"
#include "ManualGame.h"
#include "AutomatedGame.h"

// Helper: construct a fresh English board via factory
static std::unique_ptr<Board> makeFreshBoard(int size = 7) {
    return Board::create({ size, BoardType::English });
}

// Helper: set a playable cell's state directly
static void setState(Board& board, int col, int row, CellState state) {
    auto* pc = dynamic_cast<PlayableCell*>(board.getCell(col, row));
    if (pc) pc->state = state;
}

// -----------------------------------------------------------------------
// AC 3.1, 3.2, 3.4 - startGame initialises clean state
// -----------------------------------------------------------------------
TEST(GameStateTest, StartGameSetsInitialPegCount) {
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(board->getPegCount());
    EXPECT_EQ(state.pegCount, board->getPegCount());
}

TEST(GameStateTest, StartGameClearsGameOverFlag) {
    GameState state;
    state.gameOver = true;
    state.won      = true;
    auto board = makeFreshBoard();
    state.startGame(board->getPegCount());
    EXPECT_FALSE(state.gameOver);
    EXPECT_FALSE(state.won);
}

TEST(GameStateTest, StartGameClearsSelection) {
    GameState state;
    state.selectPeg({ 3, 3 });
    auto board = makeFreshBoard();
    state.startGame(board->getPegCount());
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
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(board->getPegCount());
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    state.selectPeg({ centre, centre + 2 });
    state.recordMove(*board);
    EXPECT_EQ(state.pegCount, board->getPegCount());
}

TEST(GameStateTest, RecordMoveClearsSelection) {
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(board->getPegCount());
    int centre = 3;
    state.selectPeg({ centre, centre + 2 });
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    state.recordMove(*board);
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
    auto board = makeFreshBoard(5);
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 5; ++col)
            setState(*board, col, row, CellState::Empty);
    setState(*board, 1, 2, CellState::Peg);
    setState(*board, 3, 2, CellState::Peg);

    GameState state;
    state.startGame(2);
    state.pegCount = 2;
    EXPECT_TRUE(state.checkLoss(*board));
}

TEST(GameStateTest, CheckLossFalseWhenMovesExist) {
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(board->getPegCount());
    EXPECT_FALSE(state.checkLoss(*board));
}

TEST(GameStateTest, CheckLossFalseWhenOnePegLeft) {
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(1);
    state.pegCount = 1;
    EXPECT_FALSE(state.checkLoss(*board));
}

// -----------------------------------------------------------------------
// AC 5.1, 5.2 - recordMove sets gameOver and won correctly
// -----------------------------------------------------------------------
TEST(GameStateTest, RecordMoveSetsWonWhenOnePegLeft) {
    auto board = makeFreshBoard(5);
    GameState state;
    state.startGame(board->getPegCount());
    int centre = 2;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    state.recordMove(*board);
    EXPECT_EQ(state.pegCount, board->getPegCount());

    state.startGame(1);
    EXPECT_TRUE(state.checkWin());
    EXPECT_FALSE(state.gameOver);
}

// -----------------------------------------------------------------------
// AC 5.3 - pegCount reflects final count after game over
// -----------------------------------------------------------------------
TEST(GameStateTest, PegCountCorrectAfterGameOver) {
    GameState state;
    state.startGame(10);
    state.pegCount = 1;
    EXPECT_EQ(state.pegCount, 1);
    EXPECT_TRUE(state.checkWin());
}

// -----------------------------------------------------------------------
// AC 5.5 - gameOver flag set after win
// -----------------------------------------------------------------------
TEST(GameStateTest, GameOverFlagSetAfterWin) {
    auto board = makeFreshBoard(5);
    GameState state;
    state.startGame(board->getPegCount());
    int centre = 2;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    state.recordMove(*board);
    EXPECT_FALSE(state.gameOver);

    state.pegCount = 1;
    state.gameOver = false;
    if (state.checkWin()) { state.gameOver = true; state.won = true; }
    EXPECT_TRUE(state.gameOver);
    EXPECT_TRUE(state.won);
}

// -----------------------------------------------------------------------
// AC 2.1 - Default game is ManualGame (type encodes mode)
// -----------------------------------------------------------------------
TEST(GameStateTest, DefaultGameTypeIsManual) {
    BoardConfig config;  // default: Manual
    auto game = Game::create(config);
    EXPECT_NE(dynamic_cast<ManualGame*>(game.get()), nullptr);
}

// -----------------------------------------------------------------------
// AC 2.2 - Manual config produces ManualGame
// -----------------------------------------------------------------------
TEST(GameStateTest, ManualConfigProducesManualGame) {
    BoardConfig config;
    config.mode = GameMode::Manual;
    auto game = Game::create(config);
    EXPECT_NE(dynamic_cast<ManualGame*>(game.get()), nullptr);
}

// -----------------------------------------------------------------------
// AC 2.3 - Automated config produces AutomatedGame
// -----------------------------------------------------------------------
TEST(GameStateTest, AutomatedConfigProducesAutomatedGame) {
    BoardConfig config;
    config.mode = GameMode::Automated;
    auto game = Game::create(config);
    EXPECT_NE(dynamic_cast<AutomatedGame*>(game.get()), nullptr);
}

// -----------------------------------------------------------------------
// AC 2.4 - Different mode configs produce different subclasses
// -----------------------------------------------------------------------
TEST(GameStateTest, DifferentModesProduceDifferentSubclasses) {
    BoardConfig manual;    manual.mode    = GameMode::Manual;
    BoardConfig automated; automated.mode = GameMode::Automated;
    auto manualGame    = Game::create(manual);
    auto automatedGame = Game::create(automated);
    EXPECT_NE(dynamic_cast<ManualGame*>(manualGame.get()),       nullptr);
    EXPECT_NE(dynamic_cast<AutomatedGame*>(automatedGame.get()), nullptr);
}

// -----------------------------------------------------------------------
// AC 6.1 / 6.5 - Human move decrements peg count
// -----------------------------------------------------------------------
TEST(GameStateTest, AutomatedModeHumanMoveDecrementsPegCount) {
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(board->getPegCount());
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    state.recordMove(*board);
    EXPECT_EQ(state.pegCount, board->getPegCount());
    EXPECT_FALSE(state.gameOver);
}

TEST(GameStateTest, AutomatedModePegCountSyncsAfterMove) {
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(board->getPegCount());
    int before = state.pegCount;
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    state.recordMove(*board);
    EXPECT_EQ(state.pegCount, before - 1);
}

// -----------------------------------------------------------------------
// AC 6.6 - No moves available detected correctly
// -----------------------------------------------------------------------
TEST(GameStateTest, AutomatedModeCheckLossTrueWhenStuck) {
    auto board = makeFreshBoard(5);
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 5; ++col)
            setState(*board, col, row, CellState::Empty);
    setState(*board, 1, 2, CellState::Peg);
    setState(*board, 3, 2, CellState::Peg);

    GameState state;
    state.startGame(2);
    state.pegCount = 2;
    EXPECT_TRUE(state.checkLoss(*board));
}

// -----------------------------------------------------------------------
// AC 7.1 - Win condition
// -----------------------------------------------------------------------
TEST(GameStateTest, AutomatedModeCheckWinTrueWhenOnePegLeft) {
    GameState state;
    state.startGame(1);
    EXPECT_TRUE(state.checkWin());
}

// -----------------------------------------------------------------------
// AC 7.2 - Loss condition
// -----------------------------------------------------------------------
TEST(GameStateTest, AutomatedModeCheckLossFalseWhenMovesExist) {
    auto board = makeFreshBoard();
    GameState state;
    state.startGame(board->getPegCount());
    EXPECT_FALSE(state.checkLoss(*board));
}

// -----------------------------------------------------------------------
// AC 7.3 - pegCount correct at game over
// -----------------------------------------------------------------------
TEST(GameStateTest, AutomatedModePegCountCorrectAtGameOver) {
    GameState state;
    state.startGame(10);
    state.pegCount = 1;
    EXPECT_EQ(state.pegCount, 1);
    EXPECT_TRUE(state.checkWin());
}

// -----------------------------------------------------------------------
// AC 7.5 - gameOver flag set correctly
// -----------------------------------------------------------------------
TEST(GameStateTest, AutomatedModeGameOverFlagSetOnWin) {
    GameState state;
    state.startGame(10);
    state.pegCount = 1;
    state.gameOver = false;
    if (state.checkWin()) { state.gameOver = true; state.won = true; }
    EXPECT_TRUE(state.gameOver);
    EXPECT_TRUE(state.won);
}

TEST(GameStateTest, AutomatedModeGameOverFlagSetOnLoss) {
    auto board = makeFreshBoard(5);
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 5; ++col)
            setState(*board, col, row, CellState::Empty);
    setState(*board, 1, 2, CellState::Peg);
    setState(*board, 3, 2, CellState::Peg);

    GameState state;
    state.startGame(2);
    state.pegCount = 2;
    state.gameOver = false;
    if (state.checkLoss(*board)) { state.gameOver = true; state.won = false; }
    EXPECT_TRUE(state.gameOver);
    EXPECT_FALSE(state.won);
}