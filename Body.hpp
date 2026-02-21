#pragma once
#include <SFML/Graphics.hpp>
#include <deque>

/**
 * @brief Represents a celestial object with physical and graphical properties.
 */
class Body {
public:
    // --- PHYSICAL STATE ---
    sf::Vector2f position;
    sf::Vector2f velocity;
    float mass;
    
    // --- SFML GRAPHICS ---
    sf::CircleShape shape;
    
    // --- VISUAL TRAIL SYSTEM ---
    std::deque<sf::Vector2f> path;
    size_t maxPathLength = 5000;
    sf::Color trailColor;
    
    /**
     * @brief Initializes a celestial body
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     * @param m Physical mass (affects gravity and visual size)
     * @param c Custom color (defaults to transparent for auto-coloring)
     */
    Body(float x, float y, float m, sf::Color c = sf::Color::Transparent);
    
    /**
     * @brief Renders the body and its historical orbital trail.
     */
    void draw(sf::RenderWindow& window);
};