#pragma once
#include "Popup.h"
#include "GameState.h"

// AC 5.1–5.5 / 7.1–7.5
// Inherits overlay, panel, titleText, visibility from Popup.
class GameOverUI : public Popup {
public:
    explicit GameOverUI(const sf::Font& font);

    void show(const GameState& state);
    void hide() override;

    bool newGameRequested = false;

    void handleEvent(const sf::Event& event,
                     const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) const override;

private:
    sf::Text           pegCountText;
    sf::Text           buttonText;
    sf::RectangleShape button;

    static constexpr float PANEL_W  = 320.f;
    static constexpr float PANEL_H  = 200.f;
    static constexpr float BUTTON_W = 140.f;
    static constexpr float BUTTON_H = 40.f;

    void layoutContent(unsigned windowW = 800, unsigned windowH = 600);
    bool buttonContains(sf::Vector2f point) const;
};