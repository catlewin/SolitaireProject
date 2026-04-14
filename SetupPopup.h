#pragma once
#include <vector>
#include <string>
#include "Popup.h"
#include "RadioButton.h"
#include "BoardConfig.h"

// Pre-game configuration overlay (US1, US2, US3).
// Sprint 4: adds Record toggle (AC 7.1) and Replay Last Game button (AC 7.3).
class SetupPopup : public Popup {
public:
    explicit SetupPopup(const sf::Font& font);

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
    static constexpr float PANEL_H = 520.f;   // tall enough for all controls + spacing

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
    // Board type radio buttons (AC 1.6–1.10)
    // -----------------------------------------------------------------------
    sf::Text                            radioLabel;
    std::vector<RadioButton<BoardType>> typeButtons;
    BoardType                           selectedType = BoardType::English;

    // -----------------------------------------------------------------------
    // Game mode radio buttons (AC 2.1–2.4)
    // -----------------------------------------------------------------------
    sf::Text                           modeLabel;
    std::vector<RadioButton<GameMode>> modeButtons;
    GameMode                           selectedMode = GameMode::Manual;

    // -----------------------------------------------------------------------
    // Sprint 4 — Record toggle (AC 7.1, 7.2, 7.6)
    // -----------------------------------------------------------------------
    sf::RectangleShape recordButton;
    sf::Text           recordButtonText;
    bool               recordOn = false;       // AC 7.2: defaults off
    void               toggleRecord();
    bool               recordButtonContains(sf::Vector2f p) const;

    // -----------------------------------------------------------------------
    // Sprint 4 — Replay Last Game button (AC 7.3, 7.4, 7.6)
    // -----------------------------------------------------------------------
    sf::RectangleShape replayButton;
    sf::Text           replayButtonText;
    bool               replayAvailable = false;   // true when record file valid
    bool               replaySelected  = false;   // AC 7.4: hides config controls
    void               checkReplayAvailable();
    bool               replayButtonContains(sf::Vector2f p) const;

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

    void updateRecordButtonAppearance();
    void updateReplayButtonAppearance();
};