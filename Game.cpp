#include "Game.h"
#include <stdexcept>

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
// Font is loaded first via loadFont(), then passed to subsystems.
// Declaration order in Game.h guarantees font is constructed before
// gameOverUI and setupPopup — loadFont() ensures it is also *loaded*.
// -----------------------------------------------------------------------
Game::Game()
        : window(sf::VideoMode({ 800, 600 }), "Peg Solitaire"),
          font(loadFont("/System/Library/Fonts/Helvetica.ttc")),
          gameOverUI(font),
          setupPopup(font)
{
    window.setFramerateLimit(60);

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

            // AC 3.1, 3.2: player confirmed config
            if (setupPopup.confirmRequested) {
                startNewGame(setupPopup.getConfig());
                setupPopup.hide();
            }
            return; // block all other input while popup is open
        }

        // --- GameOverUI takes priority when visible (AC 5.5) ---
        if (gameOverUI.isVisible()) {
            gameOverUI.handleEvent(*event, window);

            // AC 5.4: player wants a new game
            if (gameOverUI.newGameRequested) {
                setupPopup.show(board.getConfig());
                gameOverUI.hide();
            }
            return; // block board input while game over overlay is active
        }

        // --- Board input (only during active game) ---
        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(mouse->position);
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

    // Convert screen position to grid coordinates
    sf::Vector2f origin  = board.getOrigin();
    float        spacing = board.getCellSpacing();
    int col = static_cast<int>(std::round((mousePos.x - origin.x) / spacing));
    int row = static_cast<int>(std::round((mousePos.y - origin.y) / spacing));

    Cell* clicked = board.getCell(col, row);

    // Clicked outside the board or on an invalid cell
    if (!clicked || !clicked->isPlayable()) {
        // AC 4.5: deselect if clicking off the board
        if (gameState.hasSelected) {
            board.clearSelection();
            gameState.clearSelection();
        }
        return;
    }

    // --- No peg currently selected ---
    if (!gameState.hasSelected) {
        // AC 4.2: clicked cell must have a peg
        if (!clicked->hasPeg()) return;

        auto moves = MoveValidator::getValidMoves(board, { col, row });

        // AC 4.2: no valid moves for this peg — do nothing
        if (moves.empty()) return;

        // AC 4.1: select peg and highlight destinations
        std::vector<sf::Vector2i> destinations;
        for (const auto& m : moves) destinations.push_back(m.to);

        gameState.selectPeg({ col, row });
        board.highlightMoves({ col, row }, destinations);
        return;
    }

    // --- A peg is already selected ---

    // AC 4.5: clicking the selected peg again deselects it
    if (sf::Vector2i{ col, row } == gameState.selectedPos) {
        board.clearSelection();
        gameState.clearSelection();
        return;
    }

    // AC 4.3: attempt the jump to a highlighted destination
    if (clicked->state == CellState::Highlighted) {
        sf::Vector2i from = gameState.selectedPos;
        sf::Vector2i to   = { col, row };
        sf::Vector2i over = { (from.x + to.x) / 2, (from.y + to.y) / 2 };

        board.applyMove(from, over, to);
        onMoveCompleted(); // AC 4.6, 5.1, 5.2
        return;
    }

    // AC 4.4: if clicked another peg, switch selection to it
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

    // AC 4.5: clicked somewhere invalid — deselect
    board.clearSelection();
    gameState.clearSelection();
}

// -----------------------------------------------------------------------
// Game flow
// -----------------------------------------------------------------------

// AC 3.1, 3.2, 3.4: initialise all subsystems for a fresh game
void Game::startNewGame(const BoardConfig& config) {
    board.reset(config);
    gameState.startGame(board.getPegCount());
}

// AC 4.6, 5.1, 5.2: called after every valid move
void Game::onMoveCompleted() {
    gameState.recordMove(board);

    if (gameState.gameOver) {
        gameOverUI.show(gameState); // AC 5.1, 5.2, 5.3
    }
}

// -----------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------
void Game::render() {
    window.clear(sf::Color::White);

    board.draw(window);

    if (gameOverUI.isVisible()) gameOverUI.draw(window);
    if (setupPopup.isVisible())  setupPopup.draw(window);

    window.display();
}