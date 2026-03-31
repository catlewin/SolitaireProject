#include "Popup.h"

Popup::Popup(const sf::Font& font,
             float panelW, float panelH,
             unsigned windowW, unsigned windowH)
        : titleText(font)
{
    // Full-screen dimmer
    overlay.setSize({ static_cast<float>(windowW),
                      static_cast<float>(windowH) });
    overlay.setFillColor(sf::Color(0, 0, 0, 160));

    // Panel
    panel.setSize({ panelW, panelH });
    panel.setFillColor(sf::Color(245, 245, 245));
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(2.f);

    titleText.setCharacterSize(22);
    titleText.setFillColor(sf::Color(40, 40, 40));

    layoutPanel(windowW, windowH);
}

void Popup::hide() {
    visible = false;
}

void Popup::draw(sf::RenderWindow& window) const {
    if (!visible) return;
    window.draw(overlay);
    window.draw(panel);
    window.draw(titleText);
}

void Popup::layoutPanel(unsigned windowW, unsigned windowH) {
    sf::Vector2f panelSize = panel.getSize();
    float px = (windowW - panelSize.x) / 2.f;
    float py = (windowH - panelSize.y) / 2.f;
    panel.setPosition({ px, py });
    titleText.setPosition({ px + PADDING, py + PADDING });
}