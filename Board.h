#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Cell.h"
#include "BoardConfig.h"

// -----------------------------------------------------------------------
// Board — abstract base class
// Handles grid construction, peg placement, move application, selection,
// drawing, and randomization. Subclasses only override isValidCell().
// Concrete subclasses: EnglishBoard, HexagonBoard, DiamondBoard
// -----------------------------------------------------------------------
class Board {
public:
    virtual ~Board() = default;

    // AC 3.1, 3.2: initialise grid and place pegs
    void init(const BoardConfig& config);

    // AC 1.5, 3.4: full reset with same config
    void reset(const BoardConfig& config);

    // AC 4.6: apply a validated move (from -> over -> to)
    void applyMove(sf::Vector2i from, sf::Vector2i over, sf::Vector2i to);

    // AC 4.1: highlight valid destinations for a selected peg
    void highlightMoves(sf::Vector2i selected,
                        const std::vector<sf::Vector2i>& destinations);

    // AC 4.5: clear all selection/highlight state
    void clearSelection();

    // Returns nullptr if out of bounds
    Cell*       getCell(int col, int row);
    const Cell* getCell(int col, int row) const;

    int           getPegCount()    const { return pegCount; }
    int           getSize()        const { return size; }
    virtual BoardType getType()    const = 0;
    BoardConfig   getConfig()      const { return { size, getType() }; }
    sf::Vector2f  getOrigin()      const { return origin; }
    float         getCellSpacing() const { return cellSpacing; }

    // AC 8.1, 8.2: shuffle peg positions, preserving peg count
    void randomizeBoard();

    void draw(sf::RenderWindow& window) const;

    // Factory: constructs the correct Board subclass for a given config
    static std::unique_ptr<Board> create(const BoardConfig& config);

protected:
    // Pure virtual — each subclass defines its own shape mask
    virtual bool isValidCell(int col, int row) const = 0;

    std::vector<std::vector<std::unique_ptr<Cell>>> grid; // grid[row][col]
    int size = 7;
    int pegCount = 0;

    sf::Vector2f origin;
    float cellSpacing = 50.f;

private:
    void buildGrid();
    void placePegs();
    sf::Vector2f cellToScreen(int col, int row) const;
    void drawCell(sf::RenderWindow& window, const Cell& cell) const;

    PlayableCell*       asPlayable(int col, int row);
    const PlayableCell* asPlayable(int col, int row) const;
};