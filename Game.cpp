#include "Game.h"
#include <stdexcept>
#include <cmath>

// -----------------------------------------------------------------------
// Internal helper — loads font before subsystems are constructed
// -----------------------------------------------------------------------
static sf::Font loadFont(const std::string& path) {
    sf::Font f;
    if (!f.openFromFile(path)) {
        throw std::runtime_error("Failed to load font: " + path);
    }
    return f;
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

    // New Game button — top-right corner, clear of the board
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
    newGameButtonText.setPosition({ 660.f + 60.f, 16.f + 18.f });

    // AC 3.1: show setup popup immediately on launch with default config
    BoardConfig defaultConfig;
    setupPopup.show(defaultConfig);
}

// -----------------------------------------------------------------------
// Main loop
// -----------------------------------------------------------------------
void Game::run() {
    while (window.isOpen()) {
        processEvents();
        render();
    }
}

// -----------------------------------------------------------------------
// Event processing
// -----------------------------------------------------------------------
void Game::processEvents() {
    while (auto event = window.pollEvent()) {

        // Window close
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        // --- SetupPopup has highest priority when visible ---
        if (setupPopup.isVisible()) {
            setupPopup.handleEvent(*event, window);

            if (setupPopup.confirmRequested) {
                startNewGame(setupPopup.getConfig());
                setupPopup.hide();
            }
            return;
        }

        // --- GameOverUI takes priority when visible (AC 5.5) ---
        if (gameOverUI.isVisible()) {
            gameOverUI.handleEvent(*event, window);

            if (gameOverUI.newGameRequested) {
                setupPopup.show(board.getConfig());
                gameOverUI.hide();
            }
            return;
        }

        // --- Mouse clicks on main game screen ---
        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(mouse->position);

                // New Game button — re-opens setup popup with current config
                if (newGameButtonContains(mousePos)) {
                    setupPopup.show(board.getConfig());
                    return;
                }

                handleClick(mousePos);
            }
        }
    }
}

// -----------------------------------------------------------------------
// Click handling (AC 4.1-4.5)
// -----------------------------------------------------------------------
void Game::handleClick(sf::Vector2f mousePos) {
    // AC 5.5: board is locked after game over
    if (gameState.gameOver) return;

    sf::Vector2f origin  = board.getOrigin();
    float        spacing = board.getCellSpacing();
    int col = static_cast<int>(std::round((mousePos.x - origin.x) / spacing));
    int row = static_cast<int>(std::round((mousePos.y - origin.y) / spacing));

    Cell* clicked = board.getCell(col, row);

    if (!clicked || !clicked->isPlayable()) {
        if (gameState.hasSelected) {
            board.clearSelection();
            gameState.clearSelection();
        }
        return;
    }

    if (!gameState.hasSelected) {
        if (!clicked->hasPeg()) return;

        auto moves = MoveValidator::getValidMoves(board, { col, row });
        if (moves.empty()) return;

        std::vector<sf::Vector2i> destinations;
        for (const auto& m : moves) destinations.push_back(m.to);

        gameState.selectPeg({ col, row });
        board.highlightMoves({ col, row }, destinations);
        return;
    }

    // AC 4.5: clicking the selected peg again deselects it
    if (sf::Vector2i{ col, row } == gameState.selectedPos) {
        board.clearSelection();
        gameState.clearSelection();
        return;
    }

    // AC 4.3: jump to highlighted destination
    if (clicked->state == CellState::Highlighted) {
        sf::Vector2i from = gameState.selectedPos;
        sf::Vector2i to   = { col, row };
        sf::Vector2i over = { (from.x + to.x) / 2, (from.y + to.y) / 2 };

        board.applyMove(from, over, to);
        onMoveCompleted();
        return;
    }

    // AC 4.4: switch selection to another peg if it has valid moves
    if (clicked->hasPeg()) {
        auto moves = MoveValidator::getValidMoves(board, { col, row });
        if (!moves.empty()) {
            std::vector<sf::Vector2i> destinations;
            for (const auto& m : moves) destinations.push_back(m.to);

            board.clearSelection();
            gameState.selectPeg({ col, row });
            board.highlightMoves({ col, row }, destinations);
            return;
        }
    }

    // AC 4.5: invalid click — deselect
    board.clearSelection();
    gameState.clearSelection();
}

// -----------------------------------------------------------------------
// Game flow
// -----------------------------------------------------------------------
void Game::startNewGame(const BoardConfig& config) {
    board.reset(config);
    gameState.startGame(board.getPegCount());
}

void Game::onMoveCompleted() {
    gameState.recordMove(board);

    if (gameState.gameOver) {
        gameOverUI.show(gameState);
    }
}

// -----------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------
void Game::render() {
    window.clear(sf::Color::White);

    board.draw(window);

    // New Game button always visible on main screen
    window.draw(newGameButton);
    window.draw(newGameButtonText);

    if (gameOverUI.isVisible()) gameOverUI.draw(window);
    if (setupPopup.isVisible())  setupPopup.draw(window);

    window.display();
}

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------
bool Game::newGameButtonContains(sf::Vector2f pos) const {
    return newGameButton.getGlobalBounds().contains(pos);
}