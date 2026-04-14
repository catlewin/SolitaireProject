#pragma once
#include <fstream>
#include <string>
#include "BoardConfig.h"
#include "MoveValidator.h"

class Board;

// -----------------------------------------------------------------------
// GameRecorder — writes a game session to game_record.txt
//
// File format:
//   MODE   <Manual|Automated>
//   BOARD  <English|Hexagon|Diamond> <size>
//   MOVE_HUMAN   <fx> <fy> <ox> <oy> <tx> <ty>
//   MOVE_COMPUTER <fx> <fy> <ox> <oy> <tx> <ty>
//   RANDOMIZE
//   SNAPSHOT <state0> <state1> ... <stateN>   (one token per playable cell, row-major)
//   END
//
// AC 1.2: file is created before the first move
// AC 1.3: header written on startRecording()
// AC 1.4: each move appended immediately after it is applied
// AC 1.6: END written and file closed on stopRecording()
// AC 3.2, 3.3: RANDOMIZE + SNAPSHOT written by recordRandomize()
// AC 5.3, 5.4, 5.5: separate MOVE_HUMAN / MOVE_COMPUTER tokens
// -----------------------------------------------------------------------
class GameRecorder {
public:
    static constexpr const char* FILE_PATH = "game_record.txt";

    // AC 1.2: opens the file and writes the header
    void startRecording(const BoardConfig& config);

    // AC 1.4 / 5.3: append a human move
    void recordHumanMove(const MoveValidator::Move& move);

    // AC 5.4: append a computer move
    void recordComputerMove(const MoveValidator::Move& move);

    // AC 3.2, 3.3: append RANDOMIZE + full board snapshot
    void recordRandomize(const Board& board);

    // AC 1.6: write END and close the file
    void stopRecording();

    bool isRecording() const { return active; }

private:
    std::ofstream file;
    bool          active = false;

    void writeMove(const std::string& tag, const MoveValidator::Move& move);
};