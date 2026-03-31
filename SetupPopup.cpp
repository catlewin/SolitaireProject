#include "SetupPopup.h"
#include <string>

static constexpr unsigned WINDOW_W = 800;
static constexpr unsigned WINDOW_H = 600;

SetupPopup::SetupPopup(const sf::Font& font)
        : Popup(font, PANEL_W, PANEL_H),
          inputLabel(font), inputText(font),
          errorText(font), radioLabel(font),
          modeLabel(font), confirmText(font)
{
    titleText.setString("Game Setup");
    titleText.setCharacterSize(22);

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

    radioLabel.setString("Board type:");
    radioLabel.setCharacterSize(15);
    radioLabel.setFillColor(sf::Color(60, 60, 60));

    modeLabel.setString("Game mode:");
    modeLabel.setCharacterSize(15);
    modeLabel.setFillColor(sf::Color(60, 60, 60));

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
    buildTypeButtons(px + PADDING, py + 165.f, font);
    buildModeButtons(px + PADDING, py + 320.f, font);
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

void SetupPopup::show(const BoardConfig& current) {
    config       = current;
    rawInput     = std::to_string(current.size);
    selectedType = current.type;
    selectedMode = current.mode;

    updateInputDisplay();
    selectType(selectedType);
    selectMode(selectedMode);
    clearError();

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

            inputFocused = inputBoxContains(pos);
            inputBox.setOutlineColor(inputFocused
                                     ? sf::Color(80, 120, 200)
                                     : sf::Color(150, 150, 150));

            // AC 1.7, 1.9, 1.10: board type selection
            for (auto& btn : typeButtons) {
                if (btn.contains(pos)) {
                    selectType(btn.value);
                    clearError();
                }
            }

            // AC 2.2, 2.3, 2.4: game mode selection
            for (auto& btn : modeButtons) {
                if (btn.contains(pos)) {
                    selectMode(btn.value);
                }
            }

            if (confirmButtonContains(pos)) {
                if (validateAndApply()) confirmRequested = true;
            }
        }
    }

    if (const auto* text = event.getIf<sf::Event::TextEntered>()) {
        if (!inputFocused) return;
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
        if (key->code == sf::Keyboard::Key::Enter && inputFocused) {
            if (validateAndApply()) confirmRequested = true;
        }
    }
}

void SetupPopup::draw(sf::RenderWindow& window) const {
    if (!visible) return;
    Popup::draw(window);   // overlay, panel, titleText

    window.draw(inputLabel);
    window.draw(inputBox);
    window.draw(inputText);
    window.draw(errorText);

    window.draw(radioLabel);
    for (const auto& btn : typeButtons) btn.draw(window);

    window.draw(modeLabel);
    for (const auto& btn : modeButtons) btn.draw(window);

    window.draw(confirmButton);
    window.draw(confirmText);
}

// -----------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------

void SetupPopup::layoutContent() {
    float px = (WINDOW_W - PANEL_W) / 2.f;
    float py = (WINDOW_H - PANEL_H) / 2.f;

    float rowY = py + 70.f;
    inputLabel.setPosition({ px + PADDING, rowY });
    inputBox.setPosition({ px + PANEL_W - PADDING - 80.f, rowY - 4.f });
    inputText.setPosition({ px + PANEL_W - PADDING - 74.f, rowY });
    errorText.setPosition({ px + PADDING, rowY + 36.f });

    radioLabel.setPosition({ px + PADDING, py + 140.f });
    modeLabel.setPosition({ px + PADDING, py + 295.f });

    float btnX = px + (PANEL_W - 120.f) / 2.f;
    float btnY = py + PANEL_H - 38.f - PADDING;
    confirmButton.setPosition({ btnX, btnY });

    sf::FloatRect cb = confirmText.getLocalBounds();
    confirmText.setOrigin({ cb.position.x + cb.size.x / 2.f,
                            cb.position.y + cb.size.y / 2.f });
    confirmText.setPosition({ btnX + 60.f, btnY + 19.f });
}

void SetupPopup::buildTypeButtons(float startX, float startY,
                                  const sf::Font& font) {
    struct Opt { BoardType type; std::string label; };
    const Opt opts[] = {
            { BoardType::English, "English" },  // AC 1.8
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
            { GameMode::Manual,    "Manual"    },  // AC 2.2
            { GameMode::Automated, "Automated" },  // AC 2.3
    };
    float y = startY;
    for (const auto& opt : opts) {
        modeButtons.emplace_back(opt.mode, opt.label, font,
                                 sf::Vector2f{ startX, y });
        y += 36.f;
    }
    selectMode(selectedMode);
}

// AC 1.9: only one board type selected at a time
void SetupPopup::selectType(BoardType type) {
    selectedType = type;
    for (auto& btn : typeButtons) {
        btn.value == type ? btn.select() : btn.deselect();
    }
    config.type = type;
}

// AC 2.4: only one mode selected at a time
void SetupPopup::selectMode(GameMode mode) {
    selectedMode = mode;
    for (auto& btn : modeButtons) {
        btn.value == mode ? btn.select() : btn.deselect();
    }
    config.mode = mode;
}

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