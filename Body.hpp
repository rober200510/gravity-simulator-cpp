#pragma once
#include <SFML/Graphics.hpp>

class Body {
public:
    // --- PHYSICAL PROPERTIES ---
    sf::Vector2f position;
    sf::Vector2f velocity;
    float mass;

    // --- GRAPHICAL REPRESENTATION ---
    sf::CircleShape shape;

    // CONSTRUCTOR
    Body(float x, float y, float m) {
        // 1. Initialize physical properties
        position.x = x;
        position.y = y;
        mass = m;

        // Start with zero velocity
        velocity.x = 0.0f;
        velocity.y = 0.0f;

        // 2. Initialize graphical properties
        shape.setRadius(m); 
        shape.setFillColor(sf::Color::White);

        // This ensures the physics position matches the visual center.
        shape.setOrigin(m, m);
        
        // Sync visual position with physical position
        shape.setPosition(position);
    }

    // Update physics (To be implemented)
    void update(float dt) {
        // Placeholder for Stage 2
    }

    // Render the body
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
};