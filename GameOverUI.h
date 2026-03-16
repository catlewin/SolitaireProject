#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h"

// Rendered as a centred overlay on top of the board (AC 5.1–5.5).
// Game controller calls show() to activate, handleEvent() each frame,
// and polls newGameRequested to know when to start a new game.
class GameOverUI {
public:
    explicit GameOverUI(const sf::Font& font);

    // AC 5.1, 5.2, 5.3: populate and show the overlay from current game state
    void show(const GameState& state);

    // AC 5.4: hide the overlay (called by Game after starting a new game)
    void hide();

    bool isVisible() const { return visible; }

    // AC 5.4: polled by Game controller each frame after handleEvent()
    bool newGameRequested = false;

    // AC 5.5: route mouse clicks — only the New Game button is interactive
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    void draw(sf::RenderWindow& window) const;

private:
    bool visible = false;

    // Overlay panel
    sf::RectangleShape panel;
    sf::RectangleShape overlay; // semi-transparent full-screen dimmer

    // Text elements
    sf::Text titleText;    // "You Win!" / "Game Over"   (AC 5.1, 5.2)
    sf::Text pegCountText; // "Pegs remaining: N"        (AC 5.3)
    sf::Text buttonText;   // "New Game"                 (AC 5.4)

    // New Game button
    sf::RectangleShape button;

    void layout(unsigned int windowWidth, unsigned int windowHeight);
    bool buttonContains(sf::Vector2f point) const;
};