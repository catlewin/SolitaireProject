#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "BoardConfig.h"

// AC 4.1, 4.6 - all states a cell can be in
enum class CellState {
    Invalid,     // outside the playable area (e.g. corners of English board)
    Empty,       // valid hole, no peg
    Peg,         // occupied by a peg
    Selected,    // AC 4.1: peg chosen by player
    Highlighted  // AC 4.1: valid jump destination
};

struct Cell {
    CellState state = CellState::Invalid;
    sf::Vector2i gridPos;  // (col, row) in the grid
    sf::Vector2f screenPos;

    bool isPlayable() const {
        return state != CellState::Invalid;
    }

    bool hasPeg() const {
        return state == CellState::Peg || state == CellState::Selected;
    }
};

class Board {
public:
    // AC 3.1, 3.2: initialise from config
    void init(const BoardConfig& config);

    // AC 1.5, 2.5, 3.4: full reset with (potentially new) config
    void reset(const BoardConfig& config);

    // AC 4.6: apply a validated move (from -> over -> to)
    void applyMove(sf::Vector2i from, sf::Vector2i over, sf::Vector2i to);

    // AC 4.1: highlight valid destinations for a selected peg
    void highlightMoves(sf::Vector2i selected, const std::vector<sf::Vector2i>& destinations);

    // AC 4.5: clear all selection/highlight state
    void clearSelection();

    // Returns nullptr if out of bounds or invalid
    Cell* getCell(int col, int row);
    const Cell* getCell(int col, int row) const;

    int getPegCount() const { return pegCount; }
    int getSize() const { return size; }
    BoardType getType() const { return type; }

    // Returns the current config — used by Game to re-open SetupPopup (AC 1.5, 2.5)
    BoardConfig getConfig() const { return { size, type }; }

    // Pixel position of the top-left of the board (for click mapping)
    sf::Vector2f getOrigin() const { return origin; }
    float getCellSpacing() const { return cellSpacing; }

    void draw(sf::RenderWindow& window) const;

private:
    std::vector<std::vector<Cell>> grid; // grid[row][col]
    int size = 7;
    BoardType type = BoardType::English;
    int pegCount = 0;

    sf::Vector2f origin;      // top-left pixel offset
    float cellSpacing = 50.f; // pixels between cell centres

    // Layout helpers
    void buildGrid();
    void placePegs();
    sf::Vector2f cellToScreen(int col, int row) const;

    // Board shape masks
    bool isValidCell_English(int col, int row) const;
    bool isValidCell_Hexagon(int col, int row) const;
    bool isValidCell_Diamond(int col, int row) const;

    // Drawing helpers
    void drawCell(sf::RenderWindow& window, const Cell& cell) const;
};