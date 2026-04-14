#include "GameReplayer.h"
#include <fstream>
#include <sstream>
#include <filesystem>

// -----------------------------------------------------------------------
// Static helper
// -----------------------------------------------------------------------
bool GameReplayer::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
GameReplayer::GameReplayer(const std::string& path) {
    valid = parse(path);
}

// -----------------------------------------------------------------------
// Event cursor
// -----------------------------------------------------------------------
bool GameReplayer::hasNext() const {
    return cursor < static_cast<int>(events.size());
}

ReplayEvent GameReplayer::next() {
    return events[cursor++];
}

// -----------------------------------------------------------------------
// Parser
// -----------------------------------------------------------------------
bool GameReplayer::parse(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        error = "Record file not found: " + path;
        return false;
    }

    bool gotMode  = false;
    bool gotBoard = false;
    bool gotEnd   = false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        // ---- Header ----
        if (token == "MODE") {
            std::string modeStr;
            ss >> modeStr;
            if (modeStr == "Automated") config.mode = GameMode::Automated;
            else                        config.mode = GameMode::Manual;
            gotMode = true;
        }
        else if (token == "BOARD") {
            std::string typeStr;
            int sz = 7;
            ss >> typeStr >> sz;
            if      (typeStr == "Hexagon") config.type = BoardType::Hexagon;
            else if (typeStr == "Diamond") config.type = BoardType::Diamond;
            else                           config.type = BoardType::English;
            config.size = sz;
            gotBoard = true;
        }

            // ---- Move events ----
        else if (token == "MOVE_HUMAN" || token == "MOVE_COMPUTER") {
            int fx, fy, ox, oy, tx, ty;
            if (!(ss >> fx >> fy >> ox >> oy >> tx >> ty)) {
                error = "Malformed MOVE line: " + line;
                return false;
            }
            MoveEvent me;
            me.move.from = { fx, fy };
            me.move.over = { ox, oy };
            me.move.to   = { tx, ty };
            me.owner = (token == "MOVE_HUMAN") ? MoveOwner::Human : MoveOwner::Computer;
            events.push_back(me);
        }

            // ---- Randomize snapshot ----
        else if (token == "RANDOMIZE") {
            // Next line must be SNAPSHOT
            std::string snapLine;
            if (!std::getline(file, snapLine)) {
                error = "RANDOMIZE missing SNAPSHOT line";
                return false;
            }
            std::istringstream snapSS(snapLine);
            std::string snapToken;
            snapSS >> snapToken;  // consume "SNAPSHOT"
            if (snapToken != "SNAPSHOT") {
                error = "Expected SNAPSHOT after RANDOMIZE, got: " + snapToken;
                return false;
            }
            RandomizeEvent re;
            char c;
            std::string cell;
            while (snapSS >> cell) {
                if (cell == "P" || cell == "E")
                    re.snapshot.push_back(cell[0]);
                else {
                    error = "Unknown cell token in SNAPSHOT: " + cell;
                    return false;
                }
            }
            events.push_back(re);
        }

        else if (token == "END") {
            gotEnd = true;
            break;
        }
    }

    if (!gotMode || !gotBoard) {
        error = "Record file missing MODE or BOARD header";
        return false;
    }
    if (!gotEnd) {
        error = "Record file missing END marker — may be incomplete";
        return false;
    }

    return true;
}