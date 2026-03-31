#include "EnglishBoard.h"

bool EnglishBoard::isValidCell(int col, int row) const {
    int cut = size / 3;
    bool inLeftCut  = col < cut;
    bool inRightCut = col >= size - cut;
    bool inTopCut   = row < cut;
    bool inBotCut   = row >= size - cut;
    if ((inLeftCut || inRightCut) && (inTopCut || inBotCut)) return false;
    return true;
}