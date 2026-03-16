#include "GameOverUI.h"
#include <string>

// -----------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------
static constexpr float PANEL_W        = 320.f;
static constexpr float PANEL_H        = 200.f;
static constexpr float BUTTON_W       = 140.f;
static constexpr float BUTTON_H       = 40.f;
static constexpr unsigned WINDOW_W    = 800;
static constexpr unsigned WINDOW_H    = 600;

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
GameOverUI::GameOverUI(const sf::Font& font)
        : titleText(font), pegCountText(font), buttonText(font)
{
    // Full-screen dimmer behind the panel (AC 5.5 feel — board still visible but locked)
    overlay.setSize({ static_cast<float>(WINDOW_W), static_cast<float>(WINDOW_H) });
    overlay.setFillColor(sf::Color(0, 0, 0, 150));

    // Panel
    panel.setSize({ PANEL_W, PANEL_H });
    panel.setFillColor(sf::Color(245, 245, 245));
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(2.f);

    // AC 5.1, 5.2: win/loss title
    titleText.setCharacterSize(28);
    titleText.setFillColor(sf::Color(40, 40, 40));

    // AC 5.3: peg count
    pegCountText.setCharacterSize(18);
    pegCountText.setFillColor(sf::Color(80, 80, 80));

    // AC 5.4: new game button
    button.setSize({ BUTTON_W, BUTTON_H });
    button.setFillColor(sf::Color(80, 120, 200));
    button.setOutlineColor(sf::Color(40, 70, 150));
    button.setOutlineThickness(1.5f);

    buttonText.setCharacterSize(16);
    buttonText.setFillColor(sf::Color::White);
    buttonText.setString("New Game");

    layout(WINDOW_W, WINDOW_H);
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

// AC 5.1, 5.2, 5.3: set content and make visible
void GameOverUI::show(const GameState& state) {
    // AC 5.1 / 5.2: set title based on outcome
    if (state.won) {
        titleText.setString("You Win!");
        titleText.setFillColor(sf::Color(40, 160, 60));
    } else {
        titleText.setString("Game Over");
        titleText.setFillColor(sf::Color(200, 60, 60));
    }

    // AC 5.3: show final peg count
    pegCountText.setString("Pegs remaining: " + std::to_string(state.pegCount));

    // Re-centre title after string change (bounds shift with content)
    sf::FloatRect panelBounds = panel.getGlobalBounds();
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin({ titleBounds.position.x + titleBounds.size.x / 2.f,
                          titleBounds.position.y });
    titleText.setPosition({ panelBounds.position.x + PANEL_W / 2.f,
                            panelBounds.position.y + 30.f });

    sf::FloatRect countBounds = pegCountText.getLocalBounds();
    pegCountText.setOrigin({ countBounds.position.x + countBounds.size.x / 2.f,
                             countBounds.position.y });
    pegCountText.setPosition({ panelBounds.position.x + PANEL_W / 2.f,
                               panelBounds.position.y + 80.f });

    newGameRequested = false;
    visible = true;
}

// AC 5.4: hide after new game starts
void GameOverUI::hide() {
    visible = false;
    newGameRequested = false;
}

// AC 5.4, 5.5: only the New Game button responds to clicks
void GameOverUI::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!visible) return;

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(mouse->position);
            if (buttonContains(mousePos)) {
                newGameRequested = true; // Game controller polls this
            }
        }
    }
}

void GameOverUI::draw(sf::RenderWindow& window) const {
    if (!visible) return;

    window.draw(overlay);  // dim the board behind
    window.draw(panel);
    window.draw(titleText);
    window.draw(pegCountText);
    window.draw(button);
    window.draw(buttonText);
}

// -----------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------

void GameOverUI::layout(unsigned int windowWidth, unsigned int windowHeight) {
    // Centre the panel in the window
    float panelX = (windowWidth  - PANEL_W) / 2.f;
    float panelY = (windowHeight - PANEL_H) / 2.f;
    panel.setPosition({ panelX, panelY });

    // Title — centred horizontally, near top of panel
    titleText.setPosition({ panelX + PANEL_W / 2.f, panelY + 30.f });

    // Peg count — below title
    pegCountText.setPosition({ panelX + PANEL_W / 2.f, panelY + 80.f });

    // New Game button — centred near bottom of panel
    float buttonX = panelX + (PANEL_W - BUTTON_W) / 2.f;
    float buttonY = panelY + PANEL_H - BUTTON_H - 24.f;
    button.setPosition({ buttonX, buttonY });

    // Button label — centred inside button
    sf::FloatRect btnBounds = buttonText.getLocalBounds();
    buttonText.setOrigin({ btnBounds.position.x + btnBounds.size.x / 2.f,
                           btnBounds.position.y + btnBounds.size.y / 2.f });
    buttonText.setPosition({ buttonX + BUTTON_W / 2.f,
                             buttonY + BUTTON_H / 2.f });
}

bool GameOverUI::buttonContains(sf::Vector2f point) const {
    return button.getGlobalBounds().contains(point);
}