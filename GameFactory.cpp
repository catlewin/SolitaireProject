#include "GameFactory.h"
#include "ManualGame.h"
#include "AutomatedGame.h"
#include "ReplayGame.h"
#include "GameReplayer.h"

namespace GameFactory {

    std::unique_ptr<Game> create(const BoardConfig& config) {
        // AC 2.2: replay flag takes priority — construct a ReplayGame
        if (config.replay) {
            GameReplayer replayer(GameReplayer::FILE_PATH);
            return std::make_unique<ReplayGame>(std::move(replayer));
        }
        if (config.mode == GameMode::Automated)
            return std::make_unique<AutomatedGame>();
        return std::make_unique<ManualGame>();
    }

} // namespace GameFactory