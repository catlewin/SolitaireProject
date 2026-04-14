#include "GameRecorder.h"
#include "Board.h"
#include "Cell.h"
#include <stdexcept>

// -----------------------------------------------------------------------
// Helpers — convert enums to file tokens
// -----------------------------------------------------------------------
static std::string modeToken(GameMode m) {
    return m == GameMode::Automated ? "Automated" : "Manual";
}

static std::string typeToken(BoardType t) {
    switch (t) {
        case BoardType::Hexagon: return "Hexagon";
        case BoardType::Diamond: return "Diamond";
        default:                 return "English";
    }
}

static char stateChar(CellState s) {
    // P = Peg, E = Empty  (Selected/Highlighted never appear mid-record)
    return (s == CellState::Peg || s == CellState::Selected) ? 'P' : 'E';
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

// AC 1.2, 1.3: open file and write header before any move
void GameRecorder::startRecording(const BoardConfig& config) {
    file.open(FILE_PATH, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        throw std::runtime_error("GameRecorder: cannot open " + std::string(FILE_PATH));

    file << "MODE  " << modeToken(config.mode) << "\n";
    file << "BOARD " << typeToken(config.type) << " " << config.size << "\n";
    file.flush();
    active = true;
}

// AC 1.4 / 5.3
void GameRecorder::recordHumanMove(const MoveValidator::Move& move) {
    writeMove("MOVE_HUMAN", move);
}

// AC 5.4
void GameRecorder::recordComputerMove(const MoveValidator::Move& move) {
    writeMove("MOVE_COMPUTER", move);
}

// AC 3.2, 3.3: write RANDOMIZE marker then full board snapshot
void GameRecorder::recordRandomize(const Board& board) {
    if (!active) return;
    file << "RANDOMIZE\n";
    file << "SNAPSHOT";
    for (int row = 0; row < board.getSize(); ++row) {
        for (int col = 0; col < board.getSize(); ++col) {
            const auto* pc = dynamic_cast<const PlayableCell*>(board.getCell(col, row));
            if (!pc) continue;                    // skip InvalidCells
            file << " " << stateChar(pc->state);
        }
    }
    file << "\n";
    file.flush();
}

// AC 1.6: close file cleanly
void GameRecorder::stopRecording() {
    if (!active) return;
    file << "END\n";
    file.close();
    active = false;
}

// -----------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------
void GameRecorder::writeMove(const std::string& tag,
                             const MoveValidator::Move& move) {
    if (!active) return;
    file << tag
         << " " << move.from.x << " " << move.from.y
         << " " << move.over.x << " " << move.over.y
         << " " << move.to.x   << " " << move.to.y
         << "\n";
    file.flush();
}