#include "GameFactory.h"
#include "Game.h"

int main() {
    BoardConfig config;  // default: Manual, English, size 7

    while (true) {
        auto game = GameFactory::create(config);

        auto result = game->run();

        if (result == Game::RunResult::ModeSwitch) {
            // Player switched mode — rebuild correct subclass with new config
            config = game->restartConfig;
        } else {
            // Window closed normally
            break;
        }
    }

    return 0;
}