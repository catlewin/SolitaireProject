#include "Game.h"
#include "ManualGame.h"
#include "AutomatedGame.h"
#include <stdexcept>
#include <cmath>

// -----------------------------------------------------------------------
// Font loader
// -----------------------------------------------------------------------
sf::Font Game::loadFont(const std::string& path) {
    sf::Font f;
    if (!f.openFromFile(path))
        throw std::runtime_error("Failed to load font: " + path);
    return f;
}

// -----------------------------------------------------------------------
// Factory
// -----------------------------------------------------------------------
std::unique_ptr<Game> Game::create(const BoardConfig& config) {
    if (config.mode == GameMode::Automated)
        return std::make_unique<AutomatedGame>();
    return std::make_unique<ManualGame>();
}

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
Game::Game()
        : window(sf::VideoMode({ 800, 600 }), "Peg Solitaire"),
          font(loadFont("/System/Library/Fonts/Helvetica.ttc")),
          gameOverUI(font),
          setupPopup(font),
          newGameButtonText(font)
{
    window.setFramerateLimit(60);

    newGameButton.setSize({ 120.f, 36.f });
    newGameButton.setPosition({ 660.f, 16.f });
    newGameButton.setFillColor(sf::Color(80, 120, 200));
    newGameButton.setOutlineColor(sf::Color(40, 70, 150));
    newGameButton.setOutlineThickness(1.5f);

    newGameButtonText.setString("New Game");
    newGameButtonText.setCharacterSize(14);
    newGameButtonText.setFillColor(sf::Color::White);
    sf::FloatRect tb = newGameButtonText.getLocalBounds();
    newGameButtonText.setOrigin({ tb.position.x + tb.size.x / 2.f,
                                  tb.position.y + tb.size.y / 2.f });
    newGameButtonText.setPosition({ 720.f, 34.f });

    BoardConfig defaultConfig;
    setupPopup.show(defaultConfig);
}

// -----------------------------------------------------------------------
// Main loop
// -----------------------------------------------------------------------
void Game::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

// -----------------------------------------------------------------------
// Event processing (shared)
// -----------------------------------------------------------------------
void Game::processEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) { window.close(); return; }

        if (setupPopup.isVisible()) {
            setupPopup.handleEvent(*event, window);
            if (setupPopup.confirmRequested) {
                startNewGame(setupPopup.getConfig());
                setupPopup.hide();
            }
            return;
        }

        if (gameOverUI.isVisible()) {
            gameOverUI.handleEvent(*event, window);
            if (gameOverUI.newGameRequested) {
                setupPopup.show(board->getConfig());
                gameOverUI.hide();
            }
            return;
        }

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                sf::Vector2f pos = window.mapPixelToCoords(mouse->position);
                if (newGameButtonContains(pos)) {
                    // board may be null before first game starts — use default config
                    BoardConfig cfg = board ? board->getConfig() : BoardConfig{};
                    setupPopup.show(cfg);
                    return;
                }
                if (board && !handleExtraClick(pos))
                    handleBoardClick(pos);
            }
        }
    }
}

// -----------------------------------------------------------------------
// Board click — shared selection / move logic (AC 4.1–4.5)
// -----------------------------------------------------------------------
void Game::handleBoardClick(sf::Vector2f mousePos) {
    if (!board || gameState.gameOver) return;

    sf::Vector2f origin  = board->getOrigin();
    float        spacing = board->getCellSpacing();
    int col = static_cast<int>(std::round((mousePos.x - origin.x) / spacing));
    int row = static_cast<int>(std::round((mousePos.y - origin.y) / spacing));

    Cell* clicked = board->getCell(col, row);

    if (!clicked || !clicked->isPlayable()) {
        if (gameState.hasSelected) { board->clearSelection(); gameState.clearSelection(); }
        return;
    }

    if (!gameState.hasSelected) {
        if (!clicked->hasPeg()) return;
        auto moves = MoveValidator::getValidMoves(*board, { col, row });
        if (moves.empty()) return;
        std::vector<sf::Vector2i> dests;
        for (const auto& m : moves) dests.push_back(m.to);
        gameState.selectPeg({ col, row });
        board->highlightMoves({ col, row }, dests);
        return;
    }

    if (sf::Vector2i{ col, row } == gameState.selectedPos) {
        board->clearSelection(); gameState.clearSelection(); return;
    }

    // Cast to PlayableCell to check Highlighted state
    const auto* pc = dynamic_cast<const PlayableCell*>(clicked);
    if (pc && pc->state == CellState::Highlighted) {
        sf::Vector2i from = gameState.selectedPos;
        sf::Vector2i to   = { col, row };
        sf::Vector2i over = { (from.x + to.x) / 2, (from.y + to.y) / 2 };
        board->applyMove(from, over, to);
        onMoveCompleted();
        return;
    }

    if (clicked->hasPeg()) {
        auto moves = MoveValidator::getValidMoves(*board, { col, row });
        if (!moves.empty()) {
            std::vector<sf::Vector2i> dests;
            for (const auto& m : moves) dests.push_back(m.to);
            board->clearSelection();
            gameState.selectPeg({ col, row });
            board->highlightMoves({ col, row }, dests);
            return;
        }
    }

    board->clearSelection(); gameState.clearSelection();
}

// -----------------------------------------------------------------------
// Shared game flow
// -----------------------------------------------------------------------
void Game::startNewGame(const BoardConfig& config) {
    // If the requested mode differs from this subclass, signal main to rebuild
    if (config.mode != currentMode()) {
        restartRequested = true;
        restartConfig    = config;
        window.close();  // exits run() loop cleanly
        return;
    }
    board = Board::create(config);
    gameState.startGame(board->getPegCount());
    onNewGameStarted();
}

void Game::onGameOver() {
    gameOverUI.show(gameState);
}

// -----------------------------------------------------------------------
// Rendering (shared)
// -----------------------------------------------------------------------
void Game::render() {
    window.clear(sf::Color::White);
    if (board) board->draw(window);
    window.draw(newGameButton);
    window.draw(newGameButtonText);
    renderExtras(window);
    if (gameOverUI.isVisible()) gameOverUI.draw(window);
    if (setupPopup.isVisible())  setupPopup.draw(window);
    window.display();
}

bool Game::newGameButtonContains(sf::Vector2f pos) const {
    return newGameButton.getGlobalBounds().contains(pos);
}