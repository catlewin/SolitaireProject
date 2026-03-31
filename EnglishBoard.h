#pragma once
#include "Board.h"

// English board: square grid with corner blocks cut out.
// Cut size = size/3; e.g. size=7 → 2×2 corners removed.
class EnglishBoard : public Board {
public:
    BoardType getType() const override { return BoardType::English; }
protected:
    bool isValidCell(int col, int row) const override;
};