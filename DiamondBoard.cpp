#include "DiamondBoard.h"
#include <cmath>

bool DiamondBoard::isValidCell(int col, int row) const {
    int half = size / 2;
    return std::abs(col - half) + std::abs(row - half) <= half;
}