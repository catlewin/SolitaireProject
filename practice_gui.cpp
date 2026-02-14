#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Checkbox {
private:
    sf::RectangleShape box;
    sf::RectangleShape checkMark;
    sf::Text label;
    bool checked = false;
    sf::Vector2f position;

public:
    // Constructs a checkbox at a given position with a label.
    Checkbox(const sf::Vector2f& pos, const std::string& labelText, const sf::Font& font)
            : position(pos), label(font) {
        // Configure outer box
        box.setSize({20.f, 20.f});
        box.setPosition(pos);
        box.setFillColor(sf::Color::White);
        box.setOutlineColor(sf::Color::Black);
        box.setOutlineThickness(2.f);

        // Configure check mark (when selected)
        checkMark.setSize({12.f, 12.f});
        checkMark.setPosition({pos.x + 4.f, pos.y + 4.f});
        checkMark.setFillColor(sf::Color::Green);

        // Configure label text
        label.setString(labelText);
        label.setCharacterSize(16);
        label.setFillColor(sf::Color::Black);
        label.setPosition({pos.x + 30.f, pos.y});
    }

    void toggle() {
        checked = !checked;
    }

    bool isChecked() const {
        return checked;
    }

    bool contains(const sf::Vector2f& point) const {
        return box.getGlobalBounds().contains(point);
    }

    // Draws the checkbox in its current state.
    void draw(sf::RenderWindow& window) const {
        window.draw(box);
        if (checked) {
            window.draw(checkMark);
        }
        window.draw(label);
    }
};

class RadioButton {
private:
    sf::CircleShape circle;
    sf::CircleShape innerCircle;
    sf::Text label;
    bool selected = false;
    sf::Vector2f position;

public:
    // Constructs a radio button at a given position with a label.
    RadioButton(const sf::Vector2f& pos, const std::string& labelText, const sf::Font& font)
            : position(pos), label(font) {
        // Outer circle
        circle.setRadius(10.f);
        circle.setPosition(pos);
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(sf::Color::Black);
        circle.setOutlineThickness(2.f);

        // Inner indicator (when selected)
        innerCircle.setRadius(6.f);
        innerCircle.setPosition({pos.x + 4.f, pos.y + 4.f});
        innerCircle.setFillColor(sf::Color::Blue);

        // Label text
        label.setString(labelText);
        label.setCharacterSize(16);
        label.setFillColor(sf::Color::Black);
        label.setPosition({pos.x + 30.f, pos.y});
    }

    void select() {
        selected = true;
    }

    void deselect() {
        selected = false;
    }

    bool isSelected() const {
        return selected;
    }

    bool contains(const sf::Vector2f& point) const {
        return circle.getGlobalBounds().contains(point);
    }

    // Draws the radio button in its current state.
    void draw(sf::RenderWindow& window) const {
        window.draw(circle);
        if (selected) {
            window.draw(innerCircle);
        }
        window.draw(label);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML Practice GUI");

    // Load font
    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        return -1; // Font loading failed
    }

    // Title text
    sf::Text title(font);  // Initialize with font
    title.setString("Practice GUI");
    title.setCharacterSize(24);
    title.setFillColor(sf::Color::Black);
    title.setPosition({250.f, 20.f});

    // Horizontal line
    sf::RectangleShape horizontalLine({700.f, 2.f});
    horizontalLine.setPosition({50.f, 60.f});
    horizontalLine.setFillColor(sf::Color::Black);

    // Section header
    sf::Text checkboxHeader(font);  // Initialize with font
    checkboxHeader.setString("Checkboxes:");
    checkboxHeader.setCharacterSize(18);
    checkboxHeader.setFillColor(sf::Color::Black);
    checkboxHeader.setPosition({50.f, 80.f});

    // Checkboxes
    std::vector<Checkbox> checkboxes;
    checkboxes.emplace_back(sf::Vector2f{70.f, 120.f}, "Box 1", font);
    checkboxes.emplace_back(sf::Vector2f{70.f, 160.f}, "Box 2", font);
    checkboxes.emplace_back(sf::Vector2f{70.f, 200.f}, "Box 3", font);

    // Vertical line
    sf::RectangleShape verticalLine({2.f, 400.f});
    verticalLine.setPosition({400.f, 80.f});
    verticalLine.setFillColor(sf::Color::Black);

    // Radio button section
    sf::Text radioHeader(font);  // Initialize with font
    radioHeader.setString("Choose One:");
    radioHeader.setCharacterSize(18);
    radioHeader.setFillColor(sf::Color::Black);
    radioHeader.setPosition({430.f, 80.f});

    // Radio buttons
    std::vector<RadioButton> radioButtons;
    radioButtons.emplace_back(sf::Vector2f{450.f, 120.f}, "Option 1", font);
    radioButtons.emplace_back(sf::Vector2f{450.f, 160.f}, "Option 2", font);
    radioButtons.emplace_back(sf::Vector2f{450.f, 200.f}, "Option 3", font);
    radioButtons[0].select(); // Select first option by default

    // Status text
    sf::Text statusText(font);  // Initialize with font
    statusText.setString("Click elements to interact");
    statusText.setCharacterSize(14);
    statusText.setFillColor(sf::Color(100, 100, 100));
    statusText.setPosition({50.f, 500.f});

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(mouseButton->position);

                    // Check checkbox clicks
                    for (auto& checkbox : checkboxes) {
                        if (checkbox.contains(mousePos)) {
                            checkbox.toggle();
                        }
                    }

                    // Check radio button clicks
                    for (size_t i = 0; i < radioButtons.size(); ++i) {
                        if (radioButtons[i].contains(mousePos)) {
                            // Deselect all, then select clicked one
                            for (auto& radio : radioButtons) {
                                radio.deselect();
                            }
                            radioButtons[i].select();
                        }
                    }
                }
            }
        }

        window.clear(sf::Color::White);

        // Draw all elements
        window.draw(title);
        window.draw(horizontalLine);
        window.draw(verticalLine);
        window.draw(checkboxHeader);
        window.draw(radioHeader);

        for (const auto& checkbox : checkboxes) {
            checkbox.draw(window);
        }

        for (const auto& radio : radioButtons) {
            radio.draw(window);
        }

        window.draw(statusText);

        window.display();
    }

    return 0;
}