#pragma once
#include <string>
#include <vector>
#include <variant>
#include "BoardConfig.h"
#include "MoveValidator.h"

// -----------------------------------------------------------------------
// ReplayEvent — one entry from the record file
// Either a move (human or computer) or a randomize snapshot.
// -----------------------------------------------------------------------

enum class MoveOwner { Human, Computer };

struct MoveEvent {
    MoveValidator::Move move;
    MoveOwner           owner;
};

struct RandomizeEvent {
    // Ordered list of cell states for every playable cell, row-major.
    // 'P' = Peg, 'E' = Empty
    std::vector<char> snapshot;
};

using ReplayEvent = std::variant<MoveEvent, RandomizeEvent>;

// -----------------------------------------------------------------------
// GameReplayer
//
// AC 2.2: reads header to reconstruct BoardConfig
// AC 2.3: exposes events one at a time via next()
// AC 2.7: isValid() false if file is missing or malformed
// AC 4.2: RandomizeEvent carries the snapshot so replay doesn't re-randomize
// AC 5.5: MoveOwner distinguishes human from computer moves
// -----------------------------------------------------------------------
class GameReplayer {
public:
    static constexpr const char* FILE_PATH = "game_record.txt";

    // Attempt to parse the record file.  Call isValid() before using.
    explicit GameReplayer(const std::string& path = FILE_PATH);

    bool              isValid()    const { return valid; }
    std::string       errorMsg()   const { return error; }
    BoardConfig       getConfig()  const { return config; }
    int               totalEvents()const { return static_cast<int>(events.size()); }

    bool         hasNext() const;
    ReplayEvent  next();          // advances internal cursor
    int          currentIndex() const { return cursor; }

    // AC 2.7: static helper — true if the file exists and parses cleanly
    static bool fileExists(const std::string& path = FILE_PATH);

private:
    bool                    valid  = false;
    std::string             error;
    BoardConfig             config;
    std::vector<ReplayEvent> events;
    int                     cursor = 0;

    bool parse(const std::string& path);
};