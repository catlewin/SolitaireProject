#pragma once
#include <string>

// AC 2.2 - valid board types
enum class BoardType {
    English,
    Hexagon,
    Diamond
};

// AC 1.3, 1.4 - validation result
enum class ConfigError {
    None,
    OutOfRange,    // AC 1.3: size not in [5, 10]
    NonNumeric     // AC 1.4: input was not a number
};

struct BoardConfig {
    int size         = 7;                  // AC 1.1: default size
    BoardType type   = BoardType::English; // AC 2.1: default type

    // AC 1.2, 1.3, 1.4 - validates size parsed from raw string input
    // Returns ConfigError::None if valid, otherwise the relevant error.
    // On error, size is reset to 7 (AC 1.3, 1.4).
    ConfigError validateAndSetSize(const std::string& input) {
        // AC 1.4: check all characters are digits (allow leading minus for clear error)
        for (char c : input) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                size = 7;
                return ConfigError::NonNumeric;
            }
        }

        int parsed = std::stoi(input);

        // AC 1.3: must be in range [5, 10]
        if (parsed < 5 || parsed > 10) {
            size = 7;
            return ConfigError::OutOfRange;
        }

        size = parsed;
        return ConfigError::None;
    }
};