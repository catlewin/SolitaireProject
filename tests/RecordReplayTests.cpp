#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "GameRecorder.h"
#include "GameReplayer.h"
#include "Board.h"
#include "Cell.h"
#include "MoveValidator.h"
#include "GameFactory.h"
#include "ReplayGame.h"
#include "ManualGame.h"
#include "AutomatedGame.h"

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

static const std::string TEST_FILE = "test_record.txt";

// Write a minimal valid record file for parser tests
static void writeFile(const std::string& content) {
    std::ofstream f(TEST_FILE);
    f << content;
}

static void removeFile() {
    std::filesystem::remove(TEST_FILE);
}

static std::unique_ptr<Board> makeBoard(int size = 7,
                                        BoardType type = BoardType::English) {
    return Board::create({ size, type });
}

static void setState(Board& board, int col, int row, CellState state) {
    auto* pc = dynamic_cast<PlayableCell*>(board.getCell(col, row));
    if (pc) pc->state = state;
}

// -----------------------------------------------------------------------
// US1 / AC 1.2 — startRecording creates the file
// -----------------------------------------------------------------------
TEST(RecordReplayTest, StartRecordingCreatesFile) {
    GameRecorder rec;
    BoardConfig cfg;
    cfg.mode = GameMode::Manual;
    rec.startRecording(cfg);
    rec.stopRecording();
    EXPECT_TRUE(std::filesystem::exists(GameRecorder::FILE_PATH));
}

// -----------------------------------------------------------------------
// US1 / AC 1.3 — header contains MODE and BOARD tokens
// -----------------------------------------------------------------------
TEST(RecordReplayTest, HeaderContainsModeAndBoard) {
    GameRecorder rec;
    BoardConfig cfg;
    cfg.mode = GameMode::Manual;
    cfg.type = BoardType::Hexagon;
    cfg.size = 9;
    rec.startRecording(cfg);
    rec.stopRecording();

    std::ifstream f(GameRecorder::FILE_PATH);
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("MODE"), std::string::npos);
    EXPECT_NE(content.find("Manual"), std::string::npos);
    EXPECT_NE(content.find("BOARD"), std::string::npos);
    EXPECT_NE(content.find("Hexagon"), std::string::npos);
    EXPECT_NE(content.find("9"), std::string::npos);
}

// -----------------------------------------------------------------------
// US1 / AC 1.4 — MOVE_HUMAN entry written correctly
// -----------------------------------------------------------------------
TEST(RecordReplayTest, RecordHumanMoveWritesEntry) {
    GameRecorder rec;
    BoardConfig cfg;
    rec.startRecording(cfg);

    MoveValidator::Move m;
    m.from = { 3, 5 };
    m.over = { 3, 4 };
    m.to   = { 3, 3 };
    rec.recordHumanMove(m);
    rec.stopRecording();

    std::ifstream f(GameRecorder::FILE_PATH);
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("MOVE_HUMAN"), std::string::npos);
    EXPECT_NE(content.find("3 5 3 4 3 3"), std::string::npos);
}

// -----------------------------------------------------------------------
// US5 / AC 5.4, 5.5 — MOVE_COMPUTER entry written and distinguishable
// -----------------------------------------------------------------------
TEST(RecordReplayTest, RecordComputerMoveWritesDistinctEntry) {
    GameRecorder rec;
    BoardConfig cfg;
    cfg.mode = GameMode::Automated;
    rec.startRecording(cfg);

    MoveValidator::Move mh, mc;
    mh.from = { 3, 5 }; mh.over = { 3, 4 }; mh.to = { 3, 3 };
    mc.from = { 1, 3 }; mc.over = { 2, 3 }; mc.to = { 3, 3 };
    rec.recordHumanMove(mh);
    rec.recordComputerMove(mc);
    rec.stopRecording();

    std::ifstream f(GameRecorder::FILE_PATH);
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("MOVE_HUMAN"),    std::string::npos);
    EXPECT_NE(content.find("MOVE_COMPUTER"), std::string::npos);
    // Tokens must be different
    EXPECT_NE(content.find("MOVE_HUMAN"), content.find("MOVE_COMPUTER"));
}

// -----------------------------------------------------------------------
// US1 / AC 1.6 — END marker written on stopRecording
// -----------------------------------------------------------------------
TEST(RecordReplayTest, StopRecordingWritesEndMarker) {
    GameRecorder rec;
    BoardConfig cfg;
    rec.startRecording(cfg);
    rec.stopRecording();

    std::ifstream f(GameRecorder::FILE_PATH);
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("END"), std::string::npos);
}

// -----------------------------------------------------------------------
// US3 / AC 3.2, 3.3 — RANDOMIZE + SNAPSHOT written with correct cell count
// -----------------------------------------------------------------------
TEST(RecordReplayTest, RecordRandomizeWritesSnapshotWithCorrectCellCount) {
    auto board = makeBoard(7, BoardType::English);
    GameRecorder rec;
    BoardConfig cfg;
    rec.startRecording(cfg);
    rec.recordRandomize(*board);
    rec.stopRecording();

    std::ifstream f(GameRecorder::FILE_PATH);
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("RANDOMIZE"), std::string::npos);
    EXPECT_NE(content.find("SNAPSHOT"),  std::string::npos);

    // Count P and E tokens on the SNAPSHOT line
    std::istringstream ss(content);
    std::string line;
    int cellCount = 0;
    while (std::getline(ss, line)) {
        if (line.rfind("SNAPSHOT", 0) == 0) {
            std::istringstream ls(line);
            std::string tok;
            ls >> tok;  // consume "SNAPSHOT"
            while (ls >> tok) {
                EXPECT_TRUE(tok == "P" || tok == "E") << "Unexpected token: " << tok;
                cellCount++;
            }
        }
    }
    // English 7x7 has 33 playable cells
    EXPECT_EQ(cellCount, 33);
}

// -----------------------------------------------------------------------
// US3 / AC 3.4 — Multiple RANDOMIZE events recorded in sequence
// -----------------------------------------------------------------------
TEST(RecordReplayTest, MultipleRandomizeEventsRecordedInSequence) {
    auto board = makeBoard(7);
    GameRecorder rec;
    BoardConfig cfg;
    rec.startRecording(cfg);
    rec.recordRandomize(*board);
    rec.recordRandomize(*board);
    rec.stopRecording();

    std::ifstream f(GameRecorder::FILE_PATH);
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());

    size_t first  = content.find("RANDOMIZE");
    size_t second = content.find("RANDOMIZE", first + 1);
    EXPECT_NE(first,  std::string::npos);
    EXPECT_NE(second, std::string::npos);
    EXPECT_LT(first, second);
}

// -----------------------------------------------------------------------
// US2 / AC 2.7 — GameReplayer reports invalid for missing file
// -----------------------------------------------------------------------
TEST(RecordReplayTest, ReplayerInvalidWhenFileMissing) {
    removeFile();
    GameReplayer rep(TEST_FILE);
    EXPECT_FALSE(rep.isValid());
    EXPECT_FALSE(rep.errorMsg().empty());
}

// -----------------------------------------------------------------------
// US2 / AC 2.7 — GameReplayer reports invalid for malformed file
// -----------------------------------------------------------------------
TEST(RecordReplayTest, ReplayerInvalidWhenMalformed) {
    writeFile("NOT A VALID FILE\n");
    GameReplayer rep(TEST_FILE);
    EXPECT_FALSE(rep.isValid());
    removeFile();
}

// -----------------------------------------------------------------------
// US2 / AC 2.7 — GameReplayer reports invalid when END is missing
// -----------------------------------------------------------------------
TEST(RecordReplayTest, ReplayerInvalidWhenEndMarkerMissing) {
    writeFile("MODE  Manual\nBOARD English 7\nMOVE_HUMAN 3 5 3 4 3 3\n");
    GameReplayer rep(TEST_FILE);
    EXPECT_FALSE(rep.isValid());
    removeFile();
}

// -----------------------------------------------------------------------
// US2 / AC 2.2 — GameReplayer reads back correct BoardConfig from header
// -----------------------------------------------------------------------
TEST(RecordReplayTest, ReplayerParsesHeaderCorrectly) {
    writeFile("MODE  Automated\nBOARD Diamond 9\nEND\n");
    GameReplayer rep(TEST_FILE);
    ASSERT_TRUE(rep.isValid()) << rep.errorMsg();
    EXPECT_EQ(rep.getConfig().mode, GameMode::Automated);
    EXPECT_EQ(rep.getConfig().type, BoardType::Diamond);
    EXPECT_EQ(rep.getConfig().size, 9);
    removeFile();
}

// -----------------------------------------------------------------------
// US2 / AC 2.8 — Round-trip: recorder → replayer → same move sequence
// -----------------------------------------------------------------------
TEST(RecordReplayTest, RoundTripMoveSequencePreserved) {
    MoveValidator::Move m1, m2;
    m1.from = { 3, 5 }; m1.over = { 3, 4 }; m1.to = { 3, 3 };
    m2.from = { 1, 3 }; m2.over = { 2, 3 }; m2.to = { 3, 3 };

    {
        GameRecorder rec;
        BoardConfig cfg;
        cfg.mode = GameMode::Manual;
        rec.startRecording(cfg);
        rec.recordHumanMove(m1);
        rec.recordHumanMove(m2);
        rec.stopRecording();
    }

    GameReplayer rep(GameRecorder::FILE_PATH);
    ASSERT_TRUE(rep.isValid()) << rep.errorMsg();
    ASSERT_EQ(rep.totalEvents(), 2);

    auto ev1 = rep.next();
    ASSERT_TRUE(std::holds_alternative<MoveEvent>(ev1));
    auto& me1 = std::get<MoveEvent>(ev1);
    EXPECT_EQ(me1.move.from, m1.from);
    EXPECT_EQ(me1.move.over, m1.over);
    EXPECT_EQ(me1.move.to,   m1.to);
    EXPECT_EQ(me1.owner, MoveOwner::Human);

    auto ev2 = rep.next();
    ASSERT_TRUE(std::holds_alternative<MoveEvent>(ev2));
    auto& me2 = std::get<MoveEvent>(ev2);
    EXPECT_EQ(me2.move.from, m2.from);
    EXPECT_EQ(me2.move.to,   m2.to);
}

// -----------------------------------------------------------------------
// US5 / AC 5.5 — Round-trip: human and computer move owners preserved
// -----------------------------------------------------------------------
TEST(RecordReplayTest, RoundTripMoveOwnerPreserved) {
    MoveValidator::Move mh, mc;
    mh.from = { 3, 5 }; mh.over = { 3, 4 }; mh.to = { 3, 3 };
    mc.from = { 5, 3 }; mc.over = { 4, 3 }; mc.to = { 3, 3 };

    {
        GameRecorder rec;
        BoardConfig cfg; cfg.mode = GameMode::Automated;
        rec.startRecording(cfg);
        rec.recordHumanMove(mh);
        rec.recordComputerMove(mc);
        rec.stopRecording();
    }

    GameReplayer rep(GameRecorder::FILE_PATH);
    ASSERT_TRUE(rep.isValid()) << rep.errorMsg();
    ASSERT_EQ(rep.totalEvents(), 2);

    auto ev1 = rep.next();
    ASSERT_TRUE(std::holds_alternative<MoveEvent>(ev1));
    EXPECT_EQ(std::get<MoveEvent>(ev1).owner, MoveOwner::Human);

    auto ev2 = rep.next();
    ASSERT_TRUE(std::holds_alternative<MoveEvent>(ev2));
    EXPECT_EQ(std::get<MoveEvent>(ev2).owner, MoveOwner::Computer);
}

// -----------------------------------------------------------------------
// US4 / AC 4.2 — Round-trip: randomize snapshot preserved exactly
// -----------------------------------------------------------------------
TEST(RecordReplayTest, RoundTripRandomizeSnapshotPreserved) {
    auto board = makeBoard(5, BoardType::English);
    // Create a distinctive board state
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            setState(*board, c, r, CellState::Empty);
    setState(*board, 1, 2, CellState::Peg);
    setState(*board, 3, 2, CellState::Peg);
    setState(*board, 2, 1, CellState::Peg);

    {
        GameRecorder rec;
        BoardConfig cfg;
        rec.startRecording(cfg);
        rec.recordRandomize(*board);
        rec.stopRecording();
    }

    GameReplayer rep(GameRecorder::FILE_PATH);
    ASSERT_TRUE(rep.isValid()) << rep.errorMsg();
    ASSERT_EQ(rep.totalEvents(), 1);

    auto ev = rep.next();
    ASSERT_TRUE(std::holds_alternative<RandomizeEvent>(ev));
    const auto& snap = std::get<RandomizeEvent>(ev).snapshot;

    // Rebuild what the snapshot should look like from the same board
    std::vector<char> expected;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c) {
            const auto* pc = dynamic_cast<const PlayableCell*>(board->getCell(c, r));
            if (!pc) continue;
            expected.push_back(pc->state == CellState::Peg ? 'P' : 'E');
        }

    EXPECT_EQ(snap, expected);
}

// -----------------------------------------------------------------------
// US2 / AC 2.8 — Applying a replayed move sequence gives same peg count
// -----------------------------------------------------------------------
TEST(RecordReplayTest, ReplayedMovesProduceSamePegCount) {
    auto board = makeBoard();
    int centre = 3;

    // First move: the standard opening jump into the centre hole
    MoveValidator::Move m1, m2;
    m1.from = { centre, centre + 2 };
    m1.over = { centre, centre + 1 };
    m1.to   = { centre, centre };
    board->applyMove(m1.from, m1.over, m1.to);

    // Find any valid second move by scanning the whole board
    bool foundSecond = false;
    for (int row = 0; row < board->getSize() && !foundSecond; ++row) {
        for (int col = 0; col < board->getSize() && !foundSecond; ++col) {
            auto moves = MoveValidator::getValidMoves(*board, { col, row });
            if (!moves.empty()) {
                m2 = moves[0];
                foundSecond = true;
            }
        }
    }
    ASSERT_TRUE(foundSecond) << "No valid second move found on board after first move";

    board->applyMove(m2.from, m2.over, m2.to);
    int expectedPegCount = board->getPegCount();

    {
        GameRecorder rec;
        BoardConfig cfg;
        rec.startRecording(cfg);
        rec.recordHumanMove(m1);
        rec.recordHumanMove(m2);
        rec.stopRecording();
    }

    // Replay on a fresh board
    GameReplayer rep(GameRecorder::FILE_PATH);
    ASSERT_TRUE(rep.isValid()) << rep.errorMsg();

    auto replayBoard = makeBoard();
    while (rep.hasNext()) {
        auto ev = rep.next();
        ASSERT_TRUE(std::holds_alternative<MoveEvent>(ev));
        const auto& me = std::get<MoveEvent>(ev);
        replayBoard->applyMove(me.move.from, me.move.over, me.move.to);
    }

    EXPECT_EQ(replayBoard->getPegCount(), expectedPegCount);
}

// -----------------------------------------------------------------------
// US3 / AC 3.6 — Snapshot peg count matches board peg count at time of record
// -----------------------------------------------------------------------
TEST(RecordReplayTest, SnapshotPegCountMatchesBoardPegCount) {
    auto board = makeBoard(7);
    int centre = 3;
    board->applyMove({ centre, centre + 2 },
                     { centre, centre + 1 },
                     { centre, centre });
    int pegsBefore = board->getPegCount();

    {
        GameRecorder rec;
        BoardConfig cfg;
        rec.startRecording(cfg);
        rec.recordRandomize(*board);
        rec.stopRecording();
    }

    GameReplayer rep(GameRecorder::FILE_PATH);
    ASSERT_TRUE(rep.isValid());
    auto ev = rep.next();
    ASSERT_TRUE(std::holds_alternative<RandomizeEvent>(ev));
    const auto& snap = std::get<RandomizeEvent>(ev).snapshot;

    int pegCount = static_cast<int>(
            std::count(snap.begin(), snap.end(), 'P'));
    EXPECT_EQ(pegCount, pegsBefore);
}

// -----------------------------------------------------------------------
// US2 / AC 2.1 — GameReplayer::fileExists returns false when no file
// -----------------------------------------------------------------------
TEST(RecordReplayTest, FileExistsFalseWhenNoFile) {
    removeFile();
    EXPECT_FALSE(GameReplayer::fileExists(TEST_FILE));
}

// -----------------------------------------------------------------------
// US2 / AC 2.1 — GameReplayer::fileExists returns true after recording
// -----------------------------------------------------------------------
TEST(RecordReplayTest, FileExistsTrueAfterRecording) {
    {
        GameRecorder rec;
        BoardConfig cfg;
        rec.startRecording(cfg);
        rec.stopRecording();
    }
    EXPECT_TRUE(GameReplayer::fileExists(GameRecorder::FILE_PATH));
}

// -----------------------------------------------------------------------
// US1 / AC 1.7 — New recording overwrites previous file
// -----------------------------------------------------------------------
TEST(RecordReplayTest, NewRecordingOverwritesPreviousFile) {
    // First recording — 1 move
    {
        GameRecorder rec;
        BoardConfig cfg;
        rec.startRecording(cfg);
        MoveValidator::Move m;
        m.from = { 3, 5 }; m.over = { 3, 4 }; m.to = { 3, 3 };
        rec.recordHumanMove(m);
        rec.stopRecording();
    }

    // Second recording — 0 moves
    {
        GameRecorder rec;
        BoardConfig cfg;
        rec.startRecording(cfg);
        rec.stopRecording();
    }

    GameReplayer rep(GameRecorder::FILE_PATH);
    ASSERT_TRUE(rep.isValid());
    // Should be 0 events — old data gone
    EXPECT_EQ(rep.totalEvents(), 0);
}

// -----------------------------------------------------------------------
// Sprint 4 / AC 2.2 — GameFactory produces ReplayGame for replay config
// -----------------------------------------------------------------------
TEST(RecordReplayTest, GameFactoryProducesReplayGameForReplayConfig) {
    // Write a minimal valid record file first
    {
        GameRecorder rec;
        BoardConfig cfg;
        rec.startRecording(cfg);
        rec.stopRecording();
    }

    BoardConfig cfg;
    cfg.replay = true;
    auto game = GameFactory::create(cfg);
    EXPECT_NE(dynamic_cast<ReplayGame*>(game.get()), nullptr);
}

// -----------------------------------------------------------------------
// Sprint 4 — GameFactory still produces ManualGame / AutomatedGame
// when replay flag is false
// -----------------------------------------------------------------------
TEST(RecordReplayTest, GameFactoryProducesManualGameWhenNoReplay) {
    BoardConfig cfg;
    cfg.replay = false;
    cfg.mode   = GameMode::Manual;
    auto game = GameFactory::create(cfg);
    EXPECT_NE(dynamic_cast<ManualGame*>(game.get()), nullptr);
}

TEST(RecordReplayTest, GameFactoryProducesAutomatedGameWhenNoReplay) {
    BoardConfig cfg;
    cfg.replay = false;
    cfg.mode   = GameMode::Automated;
    auto game = GameFactory::create(cfg);
    EXPECT_NE(dynamic_cast<AutomatedGame*>(game.get()), nullptr);
}