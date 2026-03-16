#include "Board.h"
#include <cmath>

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

void Board::init(const BoardConfig& config) {
    size  = config.size;
    type  = config.type;

    // Centre the board on an 800x600 window
    float totalWidth = (size - 1) * cellSpacing;
    origin = { (800.f - totalWidth) / 2.f, (600.f - totalWidth) / 2.f };

    buildGrid();  // mark valid/invalid cells
    placePegs();  // fill valid cells with pegs, empty the centre (AC 3.3)
}

// AC 1.5, 2.5, 3.4
void Board::reset(const BoardConfig& config) {
    grid.clear();
    pegCount = 0;
    init(config);
}

// AC 4.3, 4.6: move the peg, remove the jumped peg, update count
void Board::applyMove(sf::Vector2i from, sf::Vector2i over, sf::Vector2i to) {
    Cell* src    = getCell(from.x, from.y);
    Cell* jumped = getCell(over.x, over.y);
    Cell* dest   = getCell(to.x, to.y);

    if (!src || !jumped || !dest) return;

    src->state    = CellState::Empty;
    jumped->state = CellState::Empty;
    dest->state   = CellState::Peg;
    pegCount--;  // AC 4.6: one peg removed per move
}

// AC 4.1: mark a selected peg and its reachable destinations
void Board::highlightMoves(sf::Vector2i selected, const std::vector<sf::Vector2i>& destinations) {
    clearSelection();
    Cell* sel = getCell(selected.x, selected.y);
    if (sel) sel->state = CellState::Selected;

    for (const auto& dest : destinations) {
        Cell* c = getCell(dest.x, dest.y);
        if (c) c->state = CellState::Highlighted;
    }
}

// AC 4.5: deselect everything, restore pegs/empty states
void Board::clearSelection() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.state == CellState::Selected)    cell.state = CellState::Peg;
            if (cell.state == CellState::Highlighted) cell.state = CellState::Empty;
        }
    }
}

Cell* Board::getCell(int col, int row) {
    if (row < 0 || row >= (int)grid.size())    return nullptr;
    if (col < 0 || col >= (int)grid[row].size()) return nullptr;
    return &grid[row][col];
}

const Cell* Board::getCell(int col, int row) const {
    if (row < 0 || row >= (int)grid.size())    return nullptr;
    if (col < 0 || col >= (int)grid[row].size()) return nullptr;
    return &grid[row][col];
}

// -----------------------------------------------------------------------
// Private: grid construction
// -----------------------------------------------------------------------

void Board::buildGrid() {
    grid.assign(size, std::vector<Cell>(size));

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            Cell& cell = grid[row][col];
            cell.gridPos   = { col, row };
            cell.screenPos = cellToScreen(col, row);

            bool valid = false;
            switch (type) {
                case BoardType::English:  valid = isValidCell_English(col, row);  break;
                case BoardType::Hexagon:  valid = isValidCell_Hexagon(col, row);  break;
                case BoardType::Diamond:  valid = isValidCell_Diamond(col, row);  break;
            }
            cell.state = valid ? CellState::Empty : CellState::Invalid;
        }
    }
}

// AC 3.3: all valid cells get a peg except the centre hole
void Board::placePegs() {
    pegCount = 0;
    int centre = size / 2;

    for (auto& row : grid) {
        for (auto& cell : row) {
            if (!cell.isPlayable()) continue;

            if (cell.gridPos == sf::Vector2i{ centre, centre }) {
                cell.state = CellState::Empty; // centre starts empty
            } else {
                cell.state = CellState::Peg;
                pegCount++;
            }
        }
    }
}

sf::Vector2f Board::cellToScreen(int col, int row) const {
    return { origin.x + col * cellSpacing,
             origin.y + row * cellSpacing };
}

// -----------------------------------------------------------------------
// Private: board shape masks
// -----------------------------------------------------------------------

// English: square grid with the four corner blocks cut out.
// Corner block size scales with board size: cut = size / 3 (integer division).
// e.g. size=7 → cut=2 removes a 2×2 block from each corner.
bool Board::isValidCell_English(int col, int row) const {
    int cut = size / 3;
    bool inLeftCut  = col < cut;
    bool inRightCut = col >= size - cut;
    bool inTopCut   = row < cut;
    bool inBotCut   = row >= size - cut;

    // Discard if in a corner region
    if ((inLeftCut  || inRightCut) && (inTopCut || inBotCut)) return false;
    return true;
}

// Hexagon: cells are valid when they fall within the hexagonal band.
// Row offset keeps each row centred; valid columns narrow toward top/bottom.
bool Board::isValidCell_Hexagon(int col, int row) const {
    int half   = size / 2;
    int offset = std::abs(row - half);
    int minCol = offset / 2;
    int maxCol = size - 1 - minCol;
    return col >= minCol && col <= maxCol;
}

// Diamond (rotated square): valid when Manhattan distance from centre ≤ half.
bool Board::isValidCell_Diamond(int col, int row) const {
    int half = size / 2;
    return std::abs(col - half) + std::abs(row - half) <= half;
}

// -----------------------------------------------------------------------
// Drawing
// -----------------------------------------------------------------------

void Board::draw(sf::RenderWindow& window) const {
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell.state != CellState::Invalid) {
                drawCell(window, cell);
            }
        }
    }
}

void Board::drawCell(sf::RenderWindow& window, const Cell& cell) const {
    const float radius = cellSpacing * 0.35f;

    sf::CircleShape shape(radius);
    shape.setOrigin({ radius, radius });
    shape.setPosition(cell.screenPos);
    shape.setOutlineThickness(1.5f);

    switch (cell.state) {
        case CellState::Empty:
            shape.setFillColor(sf::Color(200, 200, 200));
            shape.setOutlineColor(sf::Color(120, 120, 120));
            break;
        case CellState::Peg:
            shape.setFillColor(sf::Color(80, 120, 200));
            shape.setOutlineColor(sf::Color(40, 70, 150));
            break;
        case CellState::Selected:
            shape.setFillColor(sf::Color(255, 200, 50));
            shape.setOutlineColor(sf::Color(200, 140, 0));
            break;
        case CellState::Highlighted:
            shape.setFillColor(sf::Color(100, 220, 120));
            shape.setOutlineColor(sf::Color(40, 160, 60));
            break;
        default:
            break;
    }

    window.draw(shape);
}