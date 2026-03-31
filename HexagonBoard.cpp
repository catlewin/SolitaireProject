#include "HexagonBoard.h"
#include <cmath>

bool HexagonBoard::isValidCell(int col, int row) const {
    int half   = size / 2;
    int offset = std::abs(row - half);
    int minCol = offset / 2;
    int maxCol = size - 1 - minCol;
    return col >= minCol && col <= maxCol;
}