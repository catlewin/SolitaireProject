#include "ManualGame.h"
#include "MoveValidator.h"

ManualGame::ManualGame() : Game(), randButtonText(font), recIndicator(font) {
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

    // AC 7.7: REC indicator
    recIndicator.setString("[REC]");
    recIndicator.setCharacterSize(13);
    recIndicator.setFillColor(sf::Color(220, 40, 40));
    recIndicator.setPosition({ 155.f, 22.f });

    // Show the initial setup popup — ReplayGame skips this by not calling it
    BoardConfig defaultConfig;
    setupPopup.show(defaultConfig);
}

// -----------------------------------------------------------------------
// AC 1.2 / 5.1: start recording when a new game begins (if flag set)
// -----------------------------------------------------------------------
void ManualGame::onNewGameStarted() {
    if (board && pendingRecord) {
        recorder.startRecording(board->getConfig());
    }
}

// -----------------------------------------------------------------------
// AC 1.4 / 5.1: record the human move, then check win/loss
// -----------------------------------------------------------------------
void ManualGame::onMoveCompleted() {
    // We need to record the move before recordMove() clears the selection.
    // The move coordinates are passed implicitly: handleBoardClick already
    // applied applyMove(from, over, to) and called onMoveCompleted().
    // We retrieve the last move via the board's state change — instead,
    // Game::handleBoardClick passes the move to applyMove and calls us;
    // to get the coordinates we intercept at the handleBoardClick level.
    // Simplest approach: ManualGame overrides nothing extra in handleBoardClick;
    // instead the recorder is fed from handleExtraClick (randomize) and from
    // the move hook below via the stored lastMove.
    if (recorder.isRecording()) {
        recorder.recordHumanMove(lastMove);
    }

    gameState.recordMove(*board);

    if (gameState.isGameOver()) {
        recorder.stopRecording();  // AC 1.6
        onGameOver();
    }
}

// AC 8.3, 8.4: randomize button visible only during active manual game
void ManualGame::renderExtras(sf::RenderWindow& win) {
    if (board && !gameState.isGameOver()) {
        win.draw(randButton);
        win.draw(randButtonText);
        // AC 7.7: show REC indicator if recording
        if (recorder.isRecording()) win.draw(recIndicator);
    }
}

bool ManualGame::handleExtraClick(sf::Vector2f pos) {
    if (!board || gameState.isGameOver()) return false;
    if (randButtonContains(pos)) {
        board->clearSelection();
        gameState.clearSelection();
        board->randomizeBoard();

        // AC 3.2, 3.3: record the full board state after shuffle
        if (recorder.isRecording()) {
            recorder.recordRandomize(*board);
        }

        // AC 8.1 + game-over: if randomized layout has no moves, end the game
        if (!MoveValidator::hasAnyMoves(*board)) {
            gameState.recordMove(*board);
            if (gameState.isGameOver()) {
                recorder.stopRecording();  // AC 1.6
                onGameOver();
            }
        }
        return true;  // click consumed
    }
    return false;
}

bool ManualGame::randButtonContains(sf::Vector2f pos) const {
    return randButton.getGlobalBounds().contains(pos);
}