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
    Body(float x, float y, float m, sf::Color c = sf::Color::Transparent) {
        // 1. Initialize physical properties
        position.x = x;
        position.y = y;
        mass = m;

        // Start with zero velocity
        velocity.x = 0.0f;
        velocity.y = 0.0f;

        // 2. Visual Logic (Separating Mass from Radius)
        float visualRadius;

        if (m >= 1000.0f) {
            visualRadius = 45.0f; // Stars

        } else if (m >= 10.0f) {
            visualRadius = m / 2.5f; // Planets

        } else {
            visualRadius = m * 0.6f; 
            if (visualRadius < 1.5f) visualRadius = 1.5f; // Asteroids: Keep them small but visible
        }

        shape.setRadius(visualRadius);
        shape.setOrigin(visualRadius, visualRadius);
        shape.setPosition(position);

        // 3. Color Logic
        // If the user provided a specific color, use it
        if (c != sf::Color::Transparent) {
            shape.setFillColor(c);
        }
        //Otherwise, use automatic logic (Mass = Temperature proxy)
        else {
            if (m >= 15000.0f)      shape.setFillColor(sf::Color(100, 150, 255)); // Blue (Super massive/Hot)
            else if (m >= 3000.0f) shape.setFillColor(sf::Color::White);         // White
            else if (m >= 1000.0f) shape.setFillColor(sf::Color::Yellow);        // Yellow
            else if (m >= 200.0f)  shape.setFillColor(sf::Color(255, 100, 50));  // Orange/Red (Dwarf)
            else                   shape.setFillColor(sf::Color(200, 200, 200)); // Asteroids (Grey)
        }
    }

    void update(float dt) {
        // Euler Integration and sync
        position = position + (velocity * dt);
        shape.setPosition(position);
    }

    // Render the body
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
};