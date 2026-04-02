#include "GameFactory.h"
#include "ManualGame.h"
#include "AutomatedGame.h"

namespace GameFactory {

    std::unique_ptr<Game> create(const BoardConfig& config) {
        if (config.mode == GameMode::Automated)
            return std::make_unique<AutomatedGame>();
        return std::make_unique<ManualGame>();
    }

} // namespace GameFactory