#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// -----------------------------------------------------------------------
// RadioButton<T> — generic single-select radio button
// T is the value type this button represents (e.g. BoardType, GameMode)
// Replaces the duplicate RadioOption and ModeOption structs in SetupPopup
// -----------------------------------------------------------------------
template <typename T>
class RadioButton {
public:
    RadioButton() = default;

    RadioButton(T val, const std::string& labelStr, const sf::Font& font,
                sf::Vector2f position)
            : value(val), label(font)
    {
        circle.setRadius(10.f);
        circle.setPosition(position);
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(sf::Color::Black);
        circle.setOutlineThickness(1.5f);

        inner.setRadius(6.f);
        inner.setPosition({ position.x + 4.f, position.y + 4.f });
        inner.setFillColor(sf::Color(80, 120, 200));

        label.setString(labelStr);
        label.setCharacterSize(15);
        label.setFillColor(sf::Color(40, 40, 40));
        label.setPosition({ position.x + 28.f, position.y + 1.f });
    }

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    bool isSelected() const { return selected; }

    bool contains(sf::Vector2f point) const {
        return circle.getGlobalBounds().contains(point);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(circle);
        if (selected) window.draw(inner);
        window.draw(label);
    }

    T value;

private:
    sf::CircleShape circle;
    sf::CircleShape inner;
    sf::Text        label;
    bool            selected = false;
};