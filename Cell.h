#pragma once
#include <SFML/Graphics.hpp>

// States only relevant to playable cells — Invalid removed from enum
enum class CellState {
    Empty,       // valid hole, no peg
    Peg,         // occupied by a peg
    Selected,    // AC 4.1: peg chosen by player
    Highlighted  // AC 4.1: valid jump destination
};

// -----------------------------------------------------------------------
// Base Cell — shared position data and interface
// -----------------------------------------------------------------------
class Cell {
public:
    sf::Vector2i gridPos;
    sf::Vector2f screenPos;

    virtual ~Cell() = default;

    virtual bool isPlayable() const = 0;
    virtual bool hasPeg()     const = 0;
};

// -----------------------------------------------------------------------
// InvalidCell — occupies corner/out-of-bounds positions
// Never interacted with by game logic
// -----------------------------------------------------------------------
class InvalidCell : public Cell {
public:
    bool isPlayable() const override { return false; }
    bool hasPeg()     const override { return false; }
};

// -----------------------------------------------------------------------
// PlayableCell — a valid hole that can hold a peg
// -----------------------------------------------------------------------
class PlayableCell : public Cell {
public:
    CellState state = CellState::Empty;

    bool isPlayable() const override { return true; }
    bool hasPeg()     const override {
        return state == CellState::Peg || state == CellState::Selected;
    }
};