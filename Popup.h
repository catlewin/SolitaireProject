#pragma once
#include <SFML/Graphics.hpp>

// -----------------------------------------------------------------------
// Popup — abstract base for all modal overlays
// Provides: full-screen dimmer, centred panel, visibility control,
//           and a standard interface for event handling and drawing.
// Concrete subclasses: SetupPopup, GameOverUI
// -----------------------------------------------------------------------
class Popup {
public:
    explicit Popup(const sf::Font& font,
                   float panelW, float panelH,
                   unsigned windowW = 800, unsigned windowH = 600);

    virtual ~Popup() = default;

    virtual void hide();
    bool isVisible() const { return visible; }

    // Subclasses implement their specific content handling
    virtual void handleEvent(const sf::Event& event,
                             const sf::RenderWindow& window) = 0;
    virtual void draw(sf::RenderWindow& window) const;

protected:
    bool visible = false;

    sf::RectangleShape overlay;  // semi-transparent full-screen dimmer
    sf::RectangleShape panel;    // centred content panel
    sf::Text           titleText;

    static constexpr float PADDING = 24.f;

    // Centres the panel in the window — called by subclass constructors
    void layoutPanel(unsigned windowW, unsigned windowH);
};