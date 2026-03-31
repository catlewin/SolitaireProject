#include "ManualGame.h"
#include "MoveValidator.h"

ManualGame::ManualGame() : Game(), randButtonText(font) {
    // AC 8.4: Randomize button — top-left corner
    randButton.setSize({ 120.f, 36.f });
    randButton.setPosition({ 20.f, 16.f });
    randButton.setFillColor(sf::Color(100, 180, 100));
    randButton.setOutlineColor(sf::Color(50, 120, 50));
    randButton.setOutlineThickness(1.5f);

    randButtonText.setString("Randomize");
    randButtonText.setCharacterSize(14);
    randButtonText.setFillColor(sf::Color::White);
    sf::FloatRect rb = randButtonText.getLocalBounds();
    randButtonText.setOrigin({ rb.position.x + rb.size.x / 2.f,
                               rb.position.y + rb.size.y / 2.f });
    randButtonText.setPosition({ 80.f, 34.f });
}

// AC 5.1, 5.2: check win/loss after human move — no computer response
void ManualGame::onMoveCompleted() {
    gameState.recordMove(*board);
    if (gameState.gameOver) onGameOver();
}

// AC 8.3, 8.4: randomize button visible only during active manual game
void ManualGame::renderExtras(sf::RenderWindow& win) {
    if (board && !gameState.gameOver) {
        win.draw(randButton);
        win.draw(randButtonText);
    }
}

bool ManualGame::handleExtraClick(sf::Vector2f pos) {
    if (!board || gameState.gameOver) return false;
    if (randButtonContains(pos)) {
        board->clearSelection();
        gameState.clearSelection();
        board->randomizeBoard();

        // AC 8.1 + game-over: if randomized layout has no moves, end the game
        if (!MoveValidator::hasAnyMoves(*board)) {
            gameState.recordMove(*board);
            if (gameState.gameOver) onGameOver();
        }
        return true;  // click consumed
    }
    return false;
}

bool ManualGame::randButtonContains(sf::Vector2f pos) const {
    return randButton.getGlobalBounds().contains(pos);
}