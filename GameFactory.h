#pragma once
#include <memory>
#include "BoardConfig.h"

class Game;

// -----------------------------------------------------------------------
// GameFactory — decouples Game base class from its subclasses
// Game::create() previously lived in Game.cpp, forcing the base to
// include ManualGame.h and AutomatedGame.h. Moving it here breaks
// that circular knowledge: the base class no longer knows its children.
// -----------------------------------------------------------------------
namespace GameFactory {
    std::unique_ptr<Game> create(const BoardConfig& config);
}