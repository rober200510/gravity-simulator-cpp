#pragma once
#include <SFML/Graphics.hpp>
#include <deque>

class Body {
public:
    // --- PHYSICAL PROPERTIES ---
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float mass;

    // --- GRAPHICAL REPRESENTATION ---
    sf::CircleShape shape;

    // --- TRAIL SYSTEM ---
    std::deque <sf::Vector2f> path; // History of positions
    size_t maxPathLength = 5000;    // How many points to remember
    sf::Color trailColor;

    // CONSTRUCTOR
    Body(float x, float y, float m, sf::Color c = sf::Color::Transparent) {
        // 1. Initialize physical properties
        position.x = x;
        position.y = y;
        mass = m;
        velocity = {0.0f, 0.0f};
        acceleration = {0.0f, 0.0f};

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

        trailColor = shape.getFillColor();
    }

    void update(float dt) {
        // Euler Integration and sync
        // 1. Record History (Trail)
        // We push the current position into the deque
        path.push_back(position);
        
        // If we remember too much, forget the oldest point
        if (path.size() > maxPathLength) {
            path.pop_front();
        }

        // 2. Physics Integration (Still Euler for now)
        // Update velocity based on the accumulated acceleration
        velocity += acceleration * dt;
        position = position + (velocity * dt);
        shape.setPosition(position);
    }

    // Render the body
    void draw(sf::RenderWindow& window) {
        // 1. Draw Trail (VertexArray is efficient for GPU)
        if (path.size() > 1) {
            sf::VertexArray lines(sf::LinesStrip, path.size());
            
            for (size_t i = 0; i < path.size(); ++i) {
                lines[i].position = path[i];
                
                // Fade out
                sf::Color c = trailColor;
                float alphaFactor = static_cast<float>(i) / path.size();
                c.a = static_cast<sf::Uint8>(255 * alphaFactor);
                lines[i].color = c;
            }

        window.draw(lines);
        // Draw body
        window.draw(shape);
        }
    }
};