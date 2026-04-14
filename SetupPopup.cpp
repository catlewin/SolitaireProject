#include "SetupPopup.h"
#include "GameReplayer.h"
#include <string>

static constexpr unsigned WINDOW_W = 800;
static constexpr unsigned WINDOW_H = 600;

SetupPopup::SetupPopup(const sf::Font& font)
        : Popup(font, PANEL_W, PANEL_H),
          inputLabel(font), inputText(font),
          errorText(font), radioLabel(font),
          modeLabel(font), confirmText(font),
          recordButtonText(font), replayButtonText(font)
{
    titleText.setString("Game Setup");
    titleText.setCharacterSize(22);

    // -----------------------------------------------------------------------
    // Board size input
    // -----------------------------------------------------------------------
    inputLabel.setString("Board size (5-10):");
    inputLabel.setCharacterSize(15);
    inputLabel.setFillColor(sf::Color(60, 60, 60));

    inputBox.setSize({ 80.f, 32.f });
    inputBox.setFillColor(sf::Color::White);
    inputBox.setOutlineColor(sf::Color(150, 150, 150));
    inputBox.setOutlineThickness(1.5f);

    inputText.setCharacterSize(15);
    inputText.setFillColor(sf::Color(40, 40, 40));

    errorText.setCharacterSize(13);
    errorText.setFillColor(sf::Color(200, 60, 60));
    errorText.setString("");

    // -----------------------------------------------------------------------
    // Board type / mode labels
    // -----------------------------------------------------------------------
    radioLabel.setString("Board type:");
    radioLabel.setCharacterSize(15);
    radioLabel.setFillColor(sf::Color(60, 60, 60));

    modeLabel.setString("Game mode:");
    modeLabel.setCharacterSize(15);
    modeLabel.setFillColor(sf::Color(60, 60, 60));

    // -----------------------------------------------------------------------
    // Record toggle button (AC 7.1)
    // -----------------------------------------------------------------------
    recordButton.setSize({ 160.f, 32.f });
    recordButton.setOutlineThickness(1.5f);

    recordButtonText.setCharacterSize(14);
    recordButtonText.setFillColor(sf::Color::White);

    // -----------------------------------------------------------------------
    // Replay Last Game button (AC 7.3)
    // -----------------------------------------------------------------------
    replayButton.setSize({ 160.f, 32.f });
    replayButton.setOutlineThickness(1.5f);

    replayButtonText.setCharacterSize(14);
    replayButtonText.setFillColor(sf::Color::White);

    // -----------------------------------------------------------------------
    // Confirm button
    // -----------------------------------------------------------------------
    confirmButton.setSize({ 120.f, 38.f });
    confirmButton.setFillColor(sf::Color(80, 120, 200));
    confirmButton.setOutlineColor(sf::Color(40, 70, 150));
    confirmButton.setOutlineThickness(1.5f);

    confirmText.setString("Confirm");
    confirmText.setCharacterSize(15);
    confirmText.setFillColor(sf::Color::White);

    layoutContent();

    float px = (WINDOW_W - PANEL_W) / 2.f;
    float py = (WINDOW_H - PANEL_H) / 2.f;
    buildTypeButtons(px + PADDING, py + 175.f, font);
    buildModeButtons(px + PADDING, py + 335.f, font);

    updateRecordButtonAppearance();
    updateReplayButtonAppearance();
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

void SetupPopup::show(const BoardConfig& current) {
    config       = current;
    rawInput     = std::to_string(current.size);
    selectedType = current.type;
    selectedMode = current.mode;

    // AC 7.2: record toggle always resets to off on open
    recordOn      = false;
    replaySelected = false;

    checkReplayAvailable();   // AC 7.3: check if file exists

    updateInputDisplay();
    selectType(selectedType);
    selectMode(selectedMode);
    clearError();
    updateRecordButtonAppearance();
    updateReplayButtonAppearance();

    confirmRequested = false;
    inputFocused     = false;
    visible          = true;
}

void SetupPopup::hide() {
    Popup::hide();
    confirmRequested = false;
}

// -----------------------------------------------------------------------
// Event handling
// -----------------------------------------------------------------------

void SetupPopup::handleEvent(const sf::Event& event,
                             const sf::RenderWindow& window) {
    if (!visible) return;

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos = window.mapPixelToCoords(mouse->position);

            // ---- Record toggle (AC 7.6: mutually exclusive with Replay) ----
            if (!replaySelected && recordButtonContains(pos)) {
                toggleRecord();
                return;
            }

            // ---- Replay button (AC 7.3, 7.6) ----
            if (replayAvailable && replayButtonContains(pos)) {
                replaySelected = !replaySelected;
                if (replaySelected) recordOn = false;   // AC 7.6
                updateRecordButtonAppearance();
                updateReplayButtonAppearance();
                return;
            }

            // ---- Config controls — hidden when replay selected (AC 7.4) ----
            if (!replaySelected) {
                inputFocused = inputBoxContains(pos);
                inputBox.setOutlineColor(inputFocused
                                         ? sf::Color(80, 120, 200)
                                         : sf::Color(150, 150, 150));

                for (auto& btn : typeButtons) {
                    if (btn.contains(pos)) { selectType(btn.value); clearError(); }
                }
                for (auto& btn : modeButtons) {
                    if (btn.contains(pos)) selectMode(btn.value);
                }
            }

            // ---- Confirm ----
            if (confirmButtonContains(pos)) {
                if (replaySelected) {
                    // AC 2.2: launch replay — set flag, bypass normal validation
                    config.replay  = true;
                    config.record  = false;
                    confirmRequested = true;
                } else {
                    if (validateAndApply()) {
                        config.record  = recordOn;
                        config.replay  = false;
                        confirmRequested = true;
                    }
                }
            }
        }
    }

    if (const auto* text = event.getIf<sf::Event::TextEntered>()) {
        if (!inputFocused || replaySelected) return;
        uint32_t c = text->unicode;
        if (c == 8) {
            if (!rawInput.empty()) { rawInput.pop_back(); clearError(); updateInputDisplay(); }
        } else if (c >= 32 && c < 128) {
            if (std::isdigit(static_cast<unsigned char>(c)) && rawInput.size() < 2) {
                rawInput += static_cast<char>(c); clearError(); updateInputDisplay();
            } else if (!std::isdigit(static_cast<unsigned char>(c))) {
                setError("Only numeric values are accepted.");
            }
        }
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter && inputFocused && !replaySelected) {
            if (validateAndApply()) {
                config.record = recordOn;
                config.replay = false;
                confirmRequested = true;
            }
        }
    }
}

// -----------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------

void SetupPopup::draw(sf::RenderWindow& window) const {
    if (!visible) return;
    Popup::draw(window);   // overlay, panel, titleText

    // AC 7.4: hide config controls when replay is selected
    if (!replaySelected) {
        window.draw(inputLabel);
        window.draw(inputBox);
        window.draw(inputText);
        window.draw(errorText);

        window.draw(radioLabel);
        for (const auto& btn : typeButtons) btn.draw(window);

        window.draw(modeLabel);
        for (const auto& btn : modeButtons) btn.draw(window);

        // AC 7.6: hide record toggle when replay is selected (drawn below)
        window.draw(recordButton);
        window.draw(recordButtonText);
    }

    // Replay button always visible when a valid file exists (AC 7.3)
    if (replayAvailable) {
        window.draw(replayButton);
        window.draw(replayButtonText);
    }

    window.draw(confirmButton);
    window.draw(confirmText);
}

// -----------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------

void SetupPopup::layoutContent() {
    float px = (WINDOW_W - PANEL_W) / 2.f;
    float py = (WINDOW_H - PANEL_H) / 2.f;

    // Board size row
    float rowY = py + 70.f;
    inputLabel.setPosition({ px + PADDING, rowY });
    inputBox.setPosition({ px + PANEL_W - PADDING - 80.f, rowY - 4.f });
    inputText.setPosition({ px + PANEL_W - PADDING - 74.f, rowY });
    errorText.setPosition({ px + PADDING, rowY + 36.f });

    // Section labels — shifted down slightly to use extra panel height
    radioLabel.setPosition({ px + PADDING, py + 150.f });
    modeLabel.setPosition({ px + PADDING, py + 310.f });

    // Record/Replay row — below mode buttons (py + 310 + 2*36 + 16 = ~398)
    float recBtnY = py + 406.f;
    recordButton.setPosition({ px + PADDING, recBtnY });
    {
        sf::FloatRect rb = recordButtonText.getLocalBounds();
        recordButtonText.setOrigin({ rb.position.x + rb.size.x / 2.f,
                                     rb.position.y + rb.size.y / 2.f });
        recordButtonText.setPosition({ px + PADDING + 80.f, recBtnY + 16.f });
    }

    // Replay button — to the right of record toggle
    replayButton.setPosition({ px + PADDING + 170.f, recBtnY });
    {
        sf::FloatRect rb = replayButtonText.getLocalBounds();
        replayButtonText.setOrigin({ rb.position.x + rb.size.x / 2.f,
                                     rb.position.y + rb.size.y / 2.f });
        replayButtonText.setPosition({ px + PADDING + 170.f + 80.f, recBtnY + 16.f });
    }

    // Confirm button — 20px below the record/replay row
    float btnX = px + (PANEL_W - 120.f) / 2.f;
    float btnY = recBtnY + 32.f + 20.f;   // record row height + gap
    confirmButton.setPosition({ btnX, btnY });
    {
        sf::FloatRect cb = confirmText.getLocalBounds();
        confirmText.setOrigin({ cb.position.x + cb.size.x / 2.f,
                                cb.position.y + cb.size.y / 2.f });
        confirmText.setPosition({ btnX + 60.f, btnY + 19.f });
    }
}

// -----------------------------------------------------------------------
// Type / mode selection
// -----------------------------------------------------------------------

void SetupPopup::buildTypeButtons(float startX, float startY,
                                  const sf::Font& font) {
    struct Opt { BoardType type; std::string label; };
    const Opt opts[] = {
            { BoardType::English, "English" },
            { BoardType::Hexagon, "Hexagon" },
            { BoardType::Diamond, "Diamond" },
    };
    float y = startY;
    for (const auto& opt : opts) {
        typeButtons.emplace_back(opt.type, opt.label, font,
                                 sf::Vector2f{ startX, y });
        y += 36.f;
    }
    selectType(selectedType);
}

void SetupPopup::buildModeButtons(float startX, float startY,
                                  const sf::Font& font) {
    struct Opt { GameMode mode; std::string label; };
    const Opt opts[] = {
            { GameMode::Manual,    "Manual"    },
            { GameMode::Automated, "Automated" },
    };
    float y = startY;
    for (const auto& opt : opts) {
        modeButtons.emplace_back(opt.mode, opt.label, font,
                                 sf::Vector2f{ startX, y });
        y += 36.f;
    }
    selectMode(selectedMode);
}

void SetupPopup::selectType(BoardType type) {
    selectedType = type;
    for (auto& btn : typeButtons)
        btn.value == type ? btn.select() : btn.deselect();
    config.type = type;
}

void SetupPopup::selectMode(GameMode mode) {
    selectedMode = mode;
    for (auto& btn : modeButtons)
        btn.value == mode ? btn.select() : btn.deselect();
    config.mode = mode;
}

// -----------------------------------------------------------------------
// Record toggle helpers (AC 7.1, 7.2, 7.6)
// -----------------------------------------------------------------------

void SetupPopup::toggleRecord() {
    recordOn = !recordOn;
    if (recordOn) replaySelected = false;   // AC 7.6
    updateRecordButtonAppearance();
    updateReplayButtonAppearance();
}

void SetupPopup::updateRecordButtonAppearance() {
    if (recordOn) {
        // Active: solid green (AC 7.1)
        recordButton.setFillColor(sf::Color(60, 160, 60));
        recordButton.setOutlineColor(sf::Color(30, 100, 30));
        recordButtonText.setString("[REC] ON");
    } else {
        // Inactive: muted grey-green
        recordButton.setFillColor(sf::Color(120, 160, 120));
        recordButton.setOutlineColor(sf::Color(80, 120, 80));
        recordButtonText.setString("[REC] OFF");
    }
    // Re-centre text after string change
    float px = (WINDOW_W - PANEL_W) / 2.f;
    float py = (WINDOW_H - PANEL_H) / 2.f;
    float recBtnY = py + 406.f;
    sf::FloatRect rb = recordButtonText.getLocalBounds();
    recordButtonText.setOrigin({ rb.position.x + rb.size.x / 2.f,
                                 rb.position.y + rb.size.y / 2.f });
    recordButtonText.setPosition({ px + PADDING + 80.f, recBtnY + 16.f });
}

// -----------------------------------------------------------------------
// Replay button helpers (AC 7.3, 7.4, 7.6)
// -----------------------------------------------------------------------

void SetupPopup::checkReplayAvailable() {
    replayAvailable = GameReplayer::fileExists(GameReplayer::FILE_PATH);
    // Also validate that the file actually parses cleanly (AC 2.7)
    if (replayAvailable) {
        GameReplayer probe;
        replayAvailable = probe.isValid();
    }
}

void SetupPopup::updateReplayButtonAppearance() {
    float px = (WINDOW_W - PANEL_W) / 2.f;
    float py = (WINDOW_H - PANEL_H) / 2.f;
    float recBtnY = py + 406.f;

    if (!replayAvailable) {
        // AC 2.7 / 7.3: greyed out
        replayButton.setFillColor(sf::Color(160, 160, 160));
        replayButton.setOutlineColor(sf::Color(120, 120, 120));
        replayButtonText.setString("No Recording");
    } else if (replaySelected) {
        // Selected state — highlighted blue
        replayButton.setFillColor(sf::Color(60, 100, 200));
        replayButton.setOutlineColor(sf::Color(30, 60, 150));
        replayButtonText.setString("[>] Replay: ON");
    } else {
        // Available but not selected
        replayButton.setFillColor(sf::Color(100, 140, 210));
        replayButton.setOutlineColor(sf::Color(60, 90, 160));
        replayButtonText.setString("[>] Replay Game");
    }
    sf::FloatRect rb = replayButtonText.getLocalBounds();
    replayButtonText.setOrigin({ rb.position.x + rb.size.x / 2.f,
                                 rb.position.y + rb.size.y / 2.f });
    replayButtonText.setPosition({ px + PADDING + 170.f + 80.f, recBtnY + 16.f });
}

// -----------------------------------------------------------------------
// Input / validation helpers
// -----------------------------------------------------------------------

void SetupPopup::updateInputDisplay() {
    inputText.setString(rawInput);
}

bool SetupPopup::validateAndApply() {
    ConfigError err = config.validateAndSetSize(rawInput);
    if (err == ConfigError::NonNumeric) {
        rawInput = "7"; updateInputDisplay();
        setError("Only numeric values are accepted."); return false;
    }
    if (err == ConfigError::OutOfRange) {
        rawInput = "7"; updateInputDisplay();
        setError("Board size must be between 5 and 10."); return false;
    }
    clearError();
    return true;
}

void SetupPopup::setError(const std::string& msg) { errorText.setString(msg); }
void SetupPopup::clearError()                      { errorText.setString(""); }

bool SetupPopup::confirmButtonContains(sf::Vector2f p) const {
    return confirmButton.getGlobalBounds().contains(p);
}
bool SetupPopup::inputBoxContains(sf::Vector2f p) const {
    return inputBox.getGlobalBounds().contains(p);
}
bool SetupPopup::recordButtonContains(sf::Vector2f p) const {
    return recordButton.getGlobalBounds().contains(p);
}
bool SetupPopup::replayButtonContains(sf::Vector2f p) const {
    return replayButton.getGlobalBounds().contains(p);
}