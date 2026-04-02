#include "GameOverUI.h"
#include <string>

GameOverUI::GameOverUI(const sf::Font& font)
        : Popup(font, PANEL_W, PANEL_H),
          pegCountText(font), buttonText(font)
{
    titleText.setCharacterSize(28);

    pegCountText.setCharacterSize(18);
    pegCountText.setFillColor(sf::Color(80, 80, 80));

    button.setSize({ BUTTON_W, BUTTON_H });
    button.setFillColor(sf::Color(80, 120, 200));
    button.setOutlineColor(sf::Color(40, 70, 150));
    button.setOutlineThickness(1.5f);

    buttonText.setCharacterSize(16);
    buttonText.setFillColor(sf::Color::White);
    buttonText.setString("New Game");

    layoutContent();
}

void GameOverUI::show(bool won, int pegCount) {
    // AC 5.1 / 5.2 / 7.1 / 7.2: title and colour by outcome
    if (won) {
        titleText.setString("You Win!");
        titleText.setFillColor(sf::Color(40, 160, 60));
    } else {
        titleText.setString("Game Over");
        titleText.setFillColor(sf::Color(200, 60, 60));
    }

    // AC 5.3 / 7.3: peg count
    pegCountText.setString("Pegs remaining: " + std::to_string(pegCount));

    // Re-centre text after string change
    sf::FloatRect pb = panel.getGlobalBounds();

    sf::FloatRect tb = titleText.getLocalBounds();
    titleText.setOrigin({ tb.position.x + tb.size.x / 2.f,
                          tb.position.y });
    titleText.setPosition({ pb.position.x + PANEL_W / 2.f,
                            pb.position.y + 30.f });

    sf::FloatRect cb = pegCountText.getLocalBounds();
    pegCountText.setOrigin({ cb.position.x + cb.size.x / 2.f,
                             cb.position.y });
    pegCountText.setPosition({ pb.position.x + PANEL_W / 2.f,
                               pb.position.y + 80.f });

    newGameRequested = false;
    visible = true;
}

void GameOverUI::hide() {
    Popup::hide();
    newGameRequested = false;
}

void GameOverUI::handleEvent(const sf::Event& event,
                             const sf::RenderWindow& window) {
    if (!visible) return;
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos = window.mapPixelToCoords(mouse->position);
            if (buttonContains(pos)) newGameRequested = true;
        }
    }
}

void GameOverUI::draw(sf::RenderWindow& window) const {
    if (!visible) return;
    Popup::draw(window);          // draws overlay, panel, titleText
    window.draw(pegCountText);
    window.draw(button);
    window.draw(buttonText);
}

void GameOverUI::layoutContent(unsigned windowW, unsigned windowH) {
    float panelX = (windowW - PANEL_W) / 2.f;
    float panelY = (windowH - PANEL_H) / 2.f;

    pegCountText.setPosition({ panelX + PANEL_W / 2.f, panelY + 80.f });

    float buttonX = panelX + (PANEL_W - BUTTON_W) / 2.f;
    float buttonY = panelY + PANEL_H - BUTTON_H - 24.f;
    button.setPosition({ buttonX, buttonY });

    sf::FloatRect bb = buttonText.getLocalBounds();
    buttonText.setOrigin({ bb.position.x + bb.size.x / 2.f,
                           bb.position.y + bb.size.y / 2.f });
    buttonText.setPosition({ buttonX + BUTTON_W / 2.f,
                             buttonY + BUTTON_H / 2.f });
}

bool GameOverUI::buttonContains(sf::Vector2f point) const {
    return button.getGlobalBounds().contains(point);
}