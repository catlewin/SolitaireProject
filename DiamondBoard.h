#pragma once
#include "Board.h"

// Diamond board: rotated square — valid when Manhattan distance
// from centre is within half the board size.
class DiamondBoard : public Board {
public:
    BoardType getType() const override { return BoardType::Diamond; }
protected:
    bool isValidCell(int col, int row) const override;
};