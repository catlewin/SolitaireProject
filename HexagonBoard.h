#pragma once
#include "Board.h"

// Hexagon board: cells valid within a hexagonal band.
// Valid columns narrow toward top and bottom rows.
class HexagonBoard : public Board {
public:
    BoardType getType() const override { return BoardType::Hexagon; }
protected:
    bool isValidCell(int col, int row) const override;
};