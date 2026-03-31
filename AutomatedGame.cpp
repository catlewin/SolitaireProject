#include "AutomatedGame.h"

AutomatedGame::AutomatedGame() : Game() {}

// AC 6.2: after each human move, schedule the computer's response
void AutomatedGame::onMoveCompleted() {
    gameState.recordMove(*board);
    if (gameState.gameOver) { onGameOver(); return; }
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

// AC 6.2: computer applies its move; check win/loss but do NOT trigger another
void AutomatedGame::applyComputerMove() {
    computerTurn = false;
    board->applyMove(pendingMove.from, pendingMove.over, pendingMove.to);
    gameState.recordMove(*board);
    if (gameState.gameOver) onGameOver();
}