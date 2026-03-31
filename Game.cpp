#include "Game.h"
#include <stdexcept>
#include <cmath>
#include <random>

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
          newGameButtonText(font),
          randButtonText(font)
{
    window.setFramerateLimit(60);

    // New Game button — top-right corner
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

    // Randomizer button — top-left corner (AC 8.4: shown in manual mode only)
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
    randButtonText.setPosition({ 20.f + 60.f, 16.f + 18.f });

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
        update();   // AC 6.2: drives computer-turn timer each frame
        render();
    }
}

// -----------------------------------------------------------------------
// Event processing
// -----------------------------------------------------------------------
void Game::processEvents() {
    while (auto event = window.pollEvent()) {

        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        // SetupPopup has highest priority when visible
        if (setupPopup.isVisible()) {
            setupPopup.handleEvent(*event, window);

            if (setupPopup.confirmRequested) {
                startNewGame(setupPopup.getConfig());
                setupPopup.hide();
            }
            return;
        }

        // GameOverUI takes priority when visible (AC 7.5)
        if (gameOverUI.isVisible()) {
            gameOverUI.handleEvent(*event, window);

            if (gameOverUI.newGameRequested) {
                setupPopup.show(board.getConfig());
                gameOverUI.hide();
            }
            return;
        }

        // Block input during computer turn (AC 6.2)
        if (computerTurn) return;

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(mouse->position);

                if (newGameButtonContains(mousePos)) {
                    setupPopup.show(board.getConfig());
                    return;
                }

                // AC 8.1: randomizer button — manual mode only, not after game over
                if (gameState.gameMode == GameMode::Manual
                    && !gameState.gameOver
                    && randButtonContains(mousePos)) {
                    board.clearSelection();
                    gameState.clearSelection();
                    board.randomizeBoard();
                    return;
                }

                handleClick(mousePos);
            }
        }
    }
}

// -----------------------------------------------------------------------
// Update — drives computer-turn delay (AC 6.2)
// -----------------------------------------------------------------------
void Game::update() {
    if (!computerTurn) return;

    if (computerClock.getElapsedTime().asSeconds() >= COMPUTER_DELAY) {
        applyComputerMove();
    }
}

// -----------------------------------------------------------------------
// Click handling (AC 4.1-4.5, 6.1, 6.3, 6.4)
// -----------------------------------------------------------------------
void Game::handleClick(sf::Vector2f mousePos) {
    // AC 7.5: board is locked after game over
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
        if (moves.empty()) return; // AC 6.3: no valid moves from this peg

        std::vector<sf::Vector2i> destinations;
        for (const auto& m : moves) destinations.push_back(m.to);

        gameState.selectPeg({ col, row });
        board.highlightMoves({ col, row }, destinations);
        return;
    }

    // AC 6.4: clicking the selected peg again deselects it
    if (sf::Vector2i{ col, row } == gameState.selectedPos) {
        board.clearSelection();
        gameState.clearSelection();
        return;
    }

    // AC 6.1: jump to highlighted destination
    if (clicked->state == CellState::Highlighted) {
        sf::Vector2i from = gameState.selectedPos;
        sf::Vector2i to   = { col, row };
        sf::Vector2i over = { (from.x + to.x) / 2, (from.y + to.y) / 2 };

        board.applyMove(from, over, to);
        onMoveCompleted(true);
        return;
    }

    // Switch selection to another peg if it has valid moves
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

    // AC 6.3: invalid click — deselect
    board.clearSelection();
    gameState.clearSelection();
}

// -----------------------------------------------------------------------
// Game flow
// -----------------------------------------------------------------------
void Game::startNewGame(const BoardConfig& config) {
    board.reset(config);
    gameState.startGame(board.getPegCount(), config.mode);
    computerTurn = false;
}

void Game::onMoveCompleted(bool humanMove) {
    gameState.recordMove(board);

    if (gameState.gameOver) {
        gameOverUI.show(gameState);
        computerTurn = false;
        return;
    }

    // AC 6.2: schedule computer response only after a human move
    if (humanMove && gameState.gameMode == GameMode::Automated) {
        triggerComputerMove();
    }
}

// -----------------------------------------------------------------------
// Automated move logic (AC 6.2, 6.6)
// -----------------------------------------------------------------------
void Game::triggerComputerMove() {
    std::vector<MoveValidator::Move> allMoves;
    for (int row = 0; row < board.getSize(); ++row) {
        for (int col = 0; col < board.getSize(); ++col) {
            auto moves = MoveValidator::getValidMoves(board, { col, row });
            allMoves.insert(allMoves.end(), moves.begin(), moves.end());
        }
    }

    if (allMoves.empty()) {
        // AC 6.6: no moves left — recordMove will have already set gameOver
        return;
    }

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<std::size_t> dist(0, allMoves.size() - 1);
    pendingMove  = allMoves[dist(rng)];
    computerTurn = true;
    computerClock.restart();
}

void Game::applyComputerMove() {
    computerTurn = false;
    board.applyMove(pendingMove.from, pendingMove.over, pendingMove.to);
    onMoveCompleted(false);  // AC 6.2: computer move — do not trigger another
}

// -----------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------
void Game::render() {
    window.clear(sf::Color::White);

    board.draw(window);

    // New Game button always visible
    window.draw(newGameButton);
    window.draw(newGameButtonText);

    // AC 8.3, 8.4: randomizer button only in manual mode during active play
    if (gameState.gameMode == GameMode::Manual && !gameState.gameOver) {
        window.draw(randButton);
        window.draw(randButtonText);
    }

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

bool Game::randButtonContains(sf::Vector2f pos) const {
    return randButton.getGlobalBounds().contains(pos);
}