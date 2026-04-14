#include "AutomatedGame.h"

AutomatedGame::AutomatedGame() : Game(), recIndicator(font) {
    recIndicator.setString("[REC]");
    recIndicator.setCharacterSize(13);
    recIndicator.setFillColor(sf::Color(220, 40, 40));
    recIndicator.setPosition({ 20.f, 22.f });

    // Show the initial setup popup — ReplayGame skips this by not calling it
    BoardConfig defaultConfig;
    setupPopup.show(defaultConfig);
}

// AC 5.1 / 1.2: start recording when a new game begins (if flag set)
void AutomatedGame::onNewGameStarted() {
    computerTurn = false;
    if (board && pendingRecord) {
        recorder.startRecording(board->getConfig());
    }
}

// AC 5.3: record human move, then schedule computer response
void AutomatedGame::onMoveCompleted() {
    if (recorder.isRecording()) {
        recorder.recordHumanMove(lastMove);
    }

    gameState.recordMove(*board);
    if (gameState.isGameOver()) {
        recorder.stopRecording();   // AC 5.6
        onGameOver();
        return;
    }
    triggerComputerMove();
}

// AC 6.2: tick each frame — fire computer move after delay
void AutomatedGame::update() {
    if (!computerTurn) return;
    if (computerClock.getElapsedTime().asSeconds() >= COMPUTER_DELAY)
        applyComputerMove();
}

void AutomatedGame::triggerComputerMove() {
    auto move = MoveValidator::pickRandomMove(*board);
    if (!move) return;   // AC 6.6: no moves — recordMove already set gameOver
    pendingMove  = *move;
    computerTurn = true;
    computerClock.restart();
}

// AC 5.4: record computer move, then check win/loss
void AutomatedGame::applyComputerMove() {
    computerTurn = false;
    board->applyMove(pendingMove.from, pendingMove.over, pendingMove.to);

    if (recorder.isRecording()) {
        recorder.recordComputerMove(pendingMove);
    }

    gameState.recordMove(*board);
    if (gameState.isGameOver()) {
        recorder.stopRecording();   // AC 1.6
        onGameOver();
    }
}

// AC 7.7: show REC indicator during active recorded game
void AutomatedGame::renderExtras(sf::RenderWindow& win) {
    if (recorder.isRecording() && board && !gameState.isGameOver()) {
        win.draw(recIndicator);
    }
}