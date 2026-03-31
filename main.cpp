#include "Game.h"

// main.cpp — entry point
//
// The Game subclass (ManualGame or AutomatedGame) is chosen by
// Game::create() based on the config mode. Because the subclass cannot
// change at runtime, when the player switches mode in the SetupPopup,
// startNewGame() closes the window and sets restartRequested + restartConfig.
// This loop then rebuilds the correct subclass and continues seamlessly.

int main() {
    BoardConfig config;  // default: Manual, English, size 7

    while (true) {
        auto game = Game::create(config);
        game->run();

        if (game->restartRequested) {
            // Player switched mode — rebuild with the new config
            config = game->restartConfig;
        } else {
            // Window was closed normally — exit
            break;
        }
    }

    return 0;
}