#include "SetupPopup.h"
#include <string>

static constexpr unsigned WINDOW_W = 800;
static constexpr unsigned WINDOW_H = 600;

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
SetupPopup::SetupPopup(const sf::Font& font)
        : titleText(font), inputLabel(font), inputText(font),
          errorText(font), radioLabel(font), confirmText(font),
          modeLabel(font)
{
    // Full-screen dimmer
    overlay.setSize({ static_cast<float>(WINDOW_W), static_cast<float>(WINDOW_H) });
    overlay.setFillColor(sf::Color(0, 0, 0, 160));

    // Panel
    panel.setSize({ PANEL_W, PANEL_H });
    panel.setFillColor(sf::Color(245, 245, 245));
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(2.f);

    // Title
    titleText.setString("Game Setup");
    titleText.setCharacterSize(22);
    titleText.setFillColor(sf::Color(40, 40, 40));

    // Size input label
    inputLabel.setString("Board size (5-10):");
    inputLabel.setCharacterSize(15);
    inputLabel.setFillColor(sf::Color(60, 60, 60));

    // Input box
    inputBox.setSize({ 80.f, 32.f });
    inputBox.setFillColor(sf::Color::White);
    inputBox.setOutlineColor(sf::Color(150, 150, 150));
    inputBox.setOutlineThickness(1.5f);

    // Input display text
    inputText.setCharacterSize(15);
    inputText.setFillColor(sf::Color(40, 40, 40));

    // AC 1.3, 1.4: error message (hidden by default)
    errorText.setCharacterSize(13);
    errorText.setFillColor(sf::Color(200, 60, 60));
    errorText.setString("");

    // Radio section label
    radioLabel.setString("Board type:");
    radioLabel.setCharacterSize(15);
    radioLabel.setFillColor(sf::Color(60, 60, 60));

    // Mode section label (US2)
    modeLabel.setString("Game mode:");
    modeLabel.setCharacterSize(15);
    modeLabel.setFillColor(sf::Color(60, 60, 60));

    // Confirm button
    confirmButton.setSize({ 120.f, 38.f });
    confirmButton.setFillColor(sf::Color(80, 120, 200));
    confirmButton.setOutlineColor(sf::Color(40, 70, 150));
    confirmButton.setOutlineThickness(1.5f);

    confirmText.setString("Confirm");
    confirmText.setCharacterSize(15);
    confirmText.setFillColor(sf::Color::White);

    layout(WINDOW_W, WINDOW_H);

    // Build radio options after layout so positions are resolved
    float px = (WINDOW_W - PANEL_W) / 2.f;
    float py = (WINDOW_H - PANEL_H) / 2.f;
    buildRadioOptions(px + PADDING, py + 165.f, font);
    buildModeOptions(px + PADDING, py + 320.f, font);
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

// AC 1.1, 2.1: pre-fill with current (or default) config
void SetupPopup::show(const BoardConfig& current) {
    config        = current;
    rawInput      = std::to_string(current.size);
    selectedType  = current.type;
    selectedMode  = current.mode;   // AC 2.4: retain current mode

    updateInputDisplay();
    selectRadio(selectedType);
    selectMode(selectedMode);
    clearError();

    confirmRequested = false;
    inputFocused     = false;
    visible          = true;
}

void SetupPopup::hide() {
    visible          = false;
    confirmRequested = false;
}

// -----------------------------------------------------------------------
// Event handling
// -----------------------------------------------------------------------

void SetupPopup::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!visible) return;

    // --- Mouse clicks ---
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos = window.mapPixelToCoords(mouse->position);

            // Focus the input box
            inputFocused = inputBoxContains(pos);
            inputBox.setOutlineColor(inputFocused
                                     ? sf::Color(80, 120, 200)
                                     : sf::Color(150, 150, 150));

            // AC 1.7, 1.9, 1.10: board type radio button selection
            for (auto& option : radioOptions) {
                if (radioContains(option, pos)) {
                    selectRadio(option.type);
                    clearError();
                }
            }

            // Game mode selection (US2 AC 2.2, 2.3)
            for (auto& option : modeOptions) {
                if (modeContains(option, pos)) {
                    selectMode(option.mode);
                }
            }

            // Confirm button
            if (confirmButtonContains(pos)) {
                if (validateAndApply()) {
                    confirmRequested = true; // Game controller picks this up
                }
            }
        }
    }

    // --- Keyboard input for size field (AC 1.2–1.4) ---
    if (const auto* text = event.getIf<sf::Event::TextEntered>()) {
        if (!inputFocused) return;

        uint32_t c = text->unicode;

        if (c == 8) {
            // Backspace
            if (!rawInput.empty()) {
                rawInput.pop_back();
                clearError();
                updateInputDisplay();
            }
        } else if (c >= 32 && c < 128) {
            // Printable ASCII — allow digits only (AC 1.4 enforced on confirm,
            // but restrict non-digits here for immediate feedback)
            if (std::isdigit(static_cast<unsigned char>(c)) && rawInput.size() < 2) {
                rawInput += static_cast<char>(c);
                clearError();
                updateInputDisplay();
            } else if (!std::isdigit(static_cast<unsigned char>(c))) {
                setError("Only numeric values are accepted.");
            }
        }
    }

    // Enter key confirms
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter && inputFocused) {
            if (validateAndApply()) {
                confirmRequested = true;
            }
        }
    }
}

void SetupPopup::draw(sf::RenderWindow& window) const {
    if (!visible) return;

    window.draw(overlay);
    window.draw(panel);
    window.draw(titleText);

    // Size input section
    window.draw(inputLabel);
    window.draw(inputBox);
    window.draw(inputText);
    window.draw(errorText);

    // Radio section
    window.draw(radioLabel);
    for (const auto& option : radioOptions) {
        window.draw(option.circle);
        if (option.selected) window.draw(option.inner);
        window.draw(option.label);
    }

    // Mode section (US2)
    window.draw(modeLabel);
    for (const auto& option : modeOptions) {
        window.draw(option.circle);
        if (option.selected) window.draw(option.inner);
        window.draw(option.label);
    }

    window.draw(confirmButton);
    window.draw(confirmText);
}

// -----------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------

void SetupPopup::layout(unsigned int windowW, unsigned int windowH) {
    float px = (windowW - PANEL_W) / 2.f;
    float py = (windowH - PANEL_H) / 2.f;
    panel.setPosition({ px, py });

    // Title
    titleText.setPosition({ px + PADDING, py + PADDING });

    // Size input row
    float rowY = py + 70.f;
    inputLabel.setPosition({ px + PADDING, rowY });
    inputBox.setPosition({ px + PANEL_W - PADDING - 80.f, rowY - 4.f });
    inputText.setPosition({ px + PANEL_W - PADDING - 74.f, rowY });

    // Error text below input row
    errorText.setPosition({ px + PADDING, rowY + 36.f });

    // Radio options — built with font so deferred to buildRadioOptions()
    radioLabel.setPosition({ px + PADDING, py + 140.f });

    // Mode options — below board type radios (3 options × 36px = 108px, start 165 → end 273, add gap)
    modeLabel.setPosition({ px + PADDING, py + 295.f });

    // Confirm button — bottom centre of panel
    float btnX = px + (PANEL_W - 120.f) / 2.f;
    float btnY = py + PANEL_H - 38.f - PADDING;
    confirmButton.setPosition({ btnX, btnY });

    sf::FloatRect cb = confirmText.getLocalBounds();
    confirmText.setOrigin({ cb.position.x + cb.size.x / 2.f,
                            cb.position.y + cb.size.y / 2.f });
    confirmText.setPosition({ btnX + 60.f, btnY + 19.f });
}

void SetupPopup::buildRadioOptions(float startX, float startY, const sf::Font& font) {
    struct Option { BoardType type; std::string label; };
    const Option opts[] = {
            { BoardType::English,  "English"  },  // AC 1.8
            { BoardType::Hexagon,  "Hexagon"  },
            { BoardType::Diamond,  "Diamond"  },
    };

    float y = startY;
    for (const auto& opt : opts) {
        // sf::Text has no default constructor in SFML 3 — construct RadioOption
        // with label initialised directly to avoid implicitly-deleted default ctor.
        radioOptions.push_back(RadioOption{
                sf::CircleShape{},
                sf::CircleShape{},
                sf::Text(font),
                opt.type,
                (opt.type == selectedType)
        });
        RadioOption& r = radioOptions.back();

        r.circle.setRadius(10.f);
        r.circle.setPosition({ startX, y });
        r.circle.setFillColor(sf::Color::White);
        r.circle.setOutlineColor(sf::Color::Black);
        r.circle.setOutlineThickness(1.5f);

        r.inner.setRadius(6.f);
        r.inner.setPosition({ startX + 4.f, y + 4.f });
        r.inner.setFillColor(sf::Color(80, 120, 200));

        r.label.setString(opt.label);
        r.label.setCharacterSize(15);
        r.label.setFillColor(sf::Color(40, 40, 40));
        r.label.setPosition({ startX + 28.f, y + 1.f });

        y += 36.f;
    }
}

// AC 1.9, 1.10: deselect all board-type radios, select the chosen type
void SetupPopup::selectRadio(BoardType type) {
    selectedType = type;
    for (auto& option : radioOptions) {
        option.selected = (option.type == type);
    }
    config.type = type;
}

// AC 2.2, 2.3: deselect all mode radios, select the chosen mode
void SetupPopup::selectMode(GameMode mode) {
    selectedMode = mode;
    for (auto& option : modeOptions) {
        option.selected = (option.mode == mode);
    }
    config.mode = mode;
}

void SetupPopup::buildModeOptions(float startX, float startY, const sf::Font& font) {
    struct Opt { GameMode mode; std::string label; };
    const Opt opts[] = {
            { GameMode::Manual,    "Manual"    },  // AC 2.2
            { GameMode::Automated, "Automated" },  // AC 2.3
    };

    float y = startY;
    for (const auto& opt : opts) {
        modeOptions.push_back(ModeOption{
                sf::CircleShape{},
                sf::CircleShape{},
                sf::Text(font),
                opt.mode,
                (opt.mode == selectedMode)
        });
        ModeOption& m = modeOptions.back();

        m.circle.setRadius(10.f);
        m.circle.setPosition({ startX, y });
        m.circle.setFillColor(sf::Color::White);
        m.circle.setOutlineColor(sf::Color::Black);
        m.circle.setOutlineThickness(1.5f);

        m.inner.setRadius(6.f);
        m.inner.setPosition({ startX + 4.f, y + 4.f });
        m.inner.setFillColor(sf::Color(80, 120, 200));

        m.label.setString(opt.label);
        m.label.setCharacterSize(15);
        m.label.setFillColor(sf::Color(40, 40, 40));
        m.label.setPosition({ startX + 28.f, y + 1.f });

        y += 36.f;
    }
}

void SetupPopup::updateInputDisplay() {
    inputText.setString(rawInput);
}

// AC 1.2–1.4: validate raw input, write to config on success
bool SetupPopup::validateAndApply() {
    ConfigError err = config.validateAndSetSize(rawInput);

    if (err == ConfigError::NonNumeric) {
        rawInput = "7";
        updateInputDisplay();
        setError("Only numeric values are accepted.");
        return false;
    }
    if (err == ConfigError::OutOfRange) {
        rawInput = "7";
        updateInputDisplay();
        setError("Board size must be between 5 and 10.");
        return false;
    }

    clearError();
    return true;
}

void SetupPopup::setError(const std::string& msg) {
    errorText.setString(msg);
}

void SetupPopup::clearError() {
    errorText.setString("");
}

bool SetupPopup::confirmButtonContains(sf::Vector2f p) const {
    return confirmButton.getGlobalBounds().contains(p);
}

bool SetupPopup::inputBoxContains(sf::Vector2f p) const {
    return inputBox.getGlobalBounds().contains(p);
}

bool SetupPopup::radioContains(const RadioOption& r, sf::Vector2f p) const {
    return r.circle.getGlobalBounds().contains(p);
}

bool SetupPopup::modeContains(const ModeOption& m, sf::Vector2f p) const {
    return m.circle.getGlobalBounds().contains(p);
}