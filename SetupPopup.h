#pragma once
#include <vector>
#include <string>
#include "Popup.h"
#include "RadioButton.h"
#include "BoardConfig.h"

// Pre-game configuration overlay (US1, US2, US3).
// Inherits overlay, panel, titleText, visibility from Popup.
class SetupPopup : public Popup {
public:
    explicit SetupPopup(const sf::Font& font);

    // AC 1.5, 2.5: show pre-filled with current config
    void show(const BoardConfig& current);
    void hide() override;

    bool confirmRequested = false;
    BoardConfig getConfig() const { return config; }

    void handleEvent(const sf::Event& event,
                     const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) const override;

private:
    BoardConfig config;

    static constexpr float PANEL_W = 380.f;
    static constexpr float PANEL_H = 400.f;

    // -----------------------------------------------------------------------
    // Board size input (AC 1.1–1.4)
    // -----------------------------------------------------------------------
    sf::RectangleShape inputBox;
    sf::Text           inputLabel;
    sf::Text           inputText;
    sf::Text           errorText;
    std::string        rawInput;
    bool               inputFocused = false;

    // -----------------------------------------------------------------------
    // Board type radio buttons — RadioButton<BoardType> (AC 1.6–1.10)
    // -----------------------------------------------------------------------
    sf::Text                           radioLabel;
    std::vector<RadioButton<BoardType>> typeButtons;
    BoardType                          selectedType = BoardType::English;

    // -----------------------------------------------------------------------
    // Game mode radio buttons — RadioButton<GameMode> (AC 2.1–2.4)
    // -----------------------------------------------------------------------
    sf::Text                           modeLabel;
    std::vector<RadioButton<GameMode>> modeButtons;
    GameMode                           selectedMode = GameMode::Manual;

    // -----------------------------------------------------------------------
    // Confirm button
    // -----------------------------------------------------------------------
    sf::RectangleShape confirmButton;
    sf::Text           confirmText;

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    void layoutContent();
    void buildTypeButtons(float startX, float startY, const sf::Font& font);
    void buildModeButtons(float startX, float startY, const sf::Font& font);
    void selectType(BoardType type);
    void selectMode(GameMode mode);
    void updateInputDisplay();
    bool validateAndApply();
    void setError(const std::string& msg);
    void clearError();
    bool confirmButtonContains(sf::Vector2f p) const;
    bool inputBoxContains(sf::Vector2f p)      const;
};