#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "BoardConfig.h"

// Pre-game configuration overlay (US1, US2, US3).
// Shown on first launch and whenever the player requests a new game.
// Game controller polls confirmRequested each frame after handleEvent().
class SetupPopup {
public:
    explicit SetupPopup(const sf::Font& font);

    // Show the popup, optionally pre-filled with an existing config (AC 1.5, 2.5)
    void show(const BoardConfig& current);
    void hide();

    bool isVisible()        const { return visible; }

    // AC 3.1, 3.2: polled by Game after confirmRequested is true
    bool confirmRequested = false;

    // Returns the validated config — only valid when confirmRequested is true
    BoardConfig getConfig() const { return config; }

    // Route keyboard + mouse events
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    void draw(sf::RenderWindow& window) const;

private:
    bool visible = false;
    BoardConfig config; // working copy, written on confirm

    // -----------------------------------------------------------------------
    // Layout constants
    // -----------------------------------------------------------------------
    static constexpr float PANEL_W  = 380.f;
    static constexpr float PANEL_H  = 320.f;
    static constexpr float PADDING  = 24.f;

    // -----------------------------------------------------------------------
    // Panel + overlay
    // -----------------------------------------------------------------------
    sf::RectangleShape overlay;
    sf::RectangleShape panel;
    sf::Text           titleText;

    // -----------------------------------------------------------------------
    // Board size input (AC 1.1–1.4)
    // -----------------------------------------------------------------------
    sf::RectangleShape inputBox;
    sf::Text           inputLabel;
    sf::Text           inputText;   // displays current raw input string
    sf::Text           errorText;   // AC 1.3, 1.4: validation error message
    std::string        rawInput;    // raw string being typed
    bool               inputFocused = false;

    // -----------------------------------------------------------------------
    // Board type radio buttons (AC 2.1–2.4)
    // -----------------------------------------------------------------------
    struct RadioOption {
        sf::CircleShape    circle;
        sf::CircleShape    inner;
        sf::Text           label;
        BoardType          type;
        bool               selected = false;
    };

    sf::Text                  radioLabel;
    std::vector<RadioOption>  radioOptions;
    BoardType                 selectedType = BoardType::English; // AC 2.1 default

    // -----------------------------------------------------------------------
    // Confirm button (AC 3.1, 3.2)
    // -----------------------------------------------------------------------
    sf::RectangleShape confirmButton;
    sf::Text           confirmText;

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    void layout(unsigned int windowW, unsigned int windowH);
    void buildRadioOptions(float startX, float startY, const sf::Font& font);
    void selectRadio(BoardType type);          // AC 2.4: deselects others
    void updateInputDisplay();                 // sync inputText to rawInput
    bool validateAndApply();                   // AC 1.2–1.4: returns false on error
    void setError(const std::string& msg);
    void clearError();
    bool confirmButtonContains(sf::Vector2f p) const;
    bool inputBoxContains(sf::Vector2f p)      const;
    bool radioContains(const RadioOption& r, sf::Vector2f p) const;
};