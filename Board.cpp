#include "Board.h"
#include "EnglishBoard.h"
#include "HexagonBoard.h"
#include "DiamondBoard.h"
#include <cmath>
#include <algorithm>
#include <random>

// -----------------------------------------------------------------------
// Factory
// -----------------------------------------------------------------------
std::unique_ptr<Board> Board::create(const BoardConfig& config) {
    std::unique_ptr<Board> board;
    switch (config.type) {
        case BoardType::English:  board = std::make_unique<EnglishBoard>();  break;
        case BoardType::Hexagon:  board = std::make_unique<HexagonBoard>();  break;
        case BoardType::Diamond:  board = std::make_unique<DiamondBoard>();  break;
    }
    board->init(config);
    return board;
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

void Board::init(const BoardConfig& config) {
    size = config.size;

    float totalWidth = (size - 1) * cellSpacing;
    origin = { (800.f - totalWidth) / 2.f, (600.f - totalWidth) / 2.f };

    buildGrid();
    placePegs();
}

void Board::reset(const BoardConfig& config) {
    grid.clear();
    pegCount = 0;
    init(config);
}

void Board::applyMove(sf::Vector2i from, sf::Vector2i over, sf::Vector2i to) {
    PlayableCell* src    = asPlayable(from.x, from.y);
    PlayableCell* jumped = asPlayable(over.x, over.y);
    PlayableCell* dest   = asPlayable(to.x,   to.y);

    if (!src || !jumped || !dest) return;

    src->state    = CellState::Empty;
    jumped->state = CellState::Empty;
    dest->state   = CellState::Peg;
    pegCount--;
}

void Board::highlightMoves(sf::Vector2i selected,
                           const std::vector<sf::Vector2i>& destinations) {
    clearSelection();
    PlayableCell* sel = asPlayable(selected.x, selected.y);
    if (sel) sel->state = CellState::Selected;

    for (const auto& dest : destinations) {
        PlayableCell* c = asPlayable(dest.x, dest.y);
        if (c) c->state = CellState::Highlighted;
    }
}

void Board::clearSelection() {
    for (auto& row : grid) {
        for (auto& cellPtr : row) {
            auto* pc = dynamic_cast<PlayableCell*>(cellPtr.get());
            if (!pc) continue;
            if (pc->state == CellState::Selected)    pc->state = CellState::Peg;
            if (pc->state == CellState::Highlighted) pc->state = CellState::Empty;
        }
    }
}

Cell* Board::getCell(int col, int row) {
    if (row < 0 || row >= (int)grid.size())         return nullptr;
    if (col < 0 || col >= (int)grid[row].size())    return nullptr;
    return grid[row][col].get();
}

const Cell* Board::getCell(int col, int row) const {
    if (row < 0 || row >= (int)grid.size())         return nullptr;
    if (col < 0 || col >= (int)grid[row].size())    return nullptr;
    return grid[row][col].get();
}

PlayableCell* Board::asPlayable(int col, int row) {
    return dynamic_cast<PlayableCell*>(getCell(col, row));
}

const PlayableCell* Board::asPlayable(int col, int row) const {
    return dynamic_cast<const PlayableCell*>(getCell(col, row));
}

// -----------------------------------------------------------------------
// AC 8.1, 8.2: shuffle peg positions among playable cells
// -----------------------------------------------------------------------
void Board::randomizeBoard() {
    std::vector<PlayableCell*> playable;
    for (auto& row : grid)
        for (auto& cellPtr : row) {
            auto* pc = dynamic_cast<PlayableCell*>(cellPtr.get());
            if (pc) playable.push_back(pc);
        }

    std::vector<CellState> states;
    states.reserve(playable.size());
    for (const auto* pc : playable) states.push_back(pc->state);

    std::mt19937 rng{ std::random_device{}() };
    std::shuffle(states.begin(), states.end(), rng);

    for (std::size_t i = 0; i < playable.size(); ++i)
        playable[i]->state = states[i];
}

// -----------------------------------------------------------------------
// Private: grid construction
// -----------------------------------------------------------------------
void Board::buildGrid() {
    grid.clear();
    grid.resize(size);

    for (int row = 0; row < size; ++row) {
        grid[row].resize(size);
        for (int col = 0; col < size; ++col) {
            if (isValidCell(col, row)) {
                auto pc = std::make_unique<PlayableCell>();
                pc->gridPos   = { col, row };
                pc->screenPos = cellToScreen(col, row);
                pc->state     = CellState::Empty;
                grid[row][col] = std::move(pc);
            } else {
                auto ic = std::make_unique<InvalidCell>();
                ic->gridPos   = { col, row };
                ic->screenPos = cellToScreen(col, row);
                grid[row][col] = std::move(ic);
            }
        }
    }
}

// AC 3.3: all playable cells get a peg except the centre
void Board::placePegs() {
    pegCount = 0;
    int centre = size / 2;

    for (auto& row : grid) {
        for (auto& cellPtr : row) {
            auto* pc = dynamic_cast<PlayableCell*>(cellPtr.get());
            if (!pc) continue;

            if (pc->gridPos == sf::Vector2i{ centre, centre }) {
                pc->state = CellState::Empty;
            } else {
                pc->state = CellState::Peg;
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
// Drawing
// -----------------------------------------------------------------------
void Board::draw(sf::RenderWindow& window) const {
    for (const auto& row : grid)
        for (const auto& cellPtr : row)
            if (cellPtr->isPlayable())
                drawCell(window, *cellPtr);
}

void Board::drawCell(sf::RenderWindow& window, const Cell& cell) const {
    const auto* pc = dynamic_cast<const PlayableCell*>(&cell);
    if (!pc) return;

    const float radius = cellSpacing * 0.35f;
    sf::CircleShape shape(radius);
    shape.setOrigin({ radius, radius });
    shape.setPosition(cell.screenPos);
    shape.setOutlineThickness(1.5f);

    switch (pc->state) {
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
    }

    window.draw(shape);
}