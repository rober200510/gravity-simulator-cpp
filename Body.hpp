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
    // Stores historical positions for orbit visualization
    std::deque<sf::Vector2f> path;
    size_t maxPathLength = 5000;
    sf::Color trailColor;
    
    /**
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     * @param m Physical mass (affects gravity and visual size)
     * @param c Custom color (defaults to transparent for auto-coloring)
     */
    Body(float x, float y, float m, sf::Color c = sf::Color::Transparent) {
        position.x = x;
        position.y = y;
        mass = m;
        velocity = {0.0f, 0.0f};
        
        // VISUAL RADIUS LOGIC
        // We decouple physical mass from visual size for display clarity
        float visualRadius;
        if (m >= 1000.0f) {
            visualRadius = 45.0f; // Massive Stars
        } else if (m >= 10.0f) {
            visualRadius = m / 2.5f; // Standard Planets
        } else {
            visualRadius = m * 0.6f; // Asteroids/Small bodies
            if (visualRadius < 1.5f) visualRadius = 1.5f; // Maintain minimum visibility
        }
        
        shape.setRadius(visualRadius);
        shape.setOrigin(visualRadius, visualRadius); // Centering the origin for physics sync
        shape.setPosition(position);
        
        // AUTOMATIC COLOR ASSIGNMENT
        // If no color is provided, we assign one based on mass (proxy for stellar classification)
        if (c != sf::Color::Transparent) {
            shape.setFillColor(c);
        } else {
            if (m >= 15000.0f) shape.setFillColor(sf::Color(100, 150, 255)); // Blue Giant
            else if (m >= 3000.0f) shape.setFillColor(sf::Color::White); // White Star
            else if (m >= 1000.0f) shape.setFillColor(sf::Color::Yellow); // G-type Star
            else if (m >= 200.0f) shape.setFillColor(sf::Color(255, 100, 50)); // Inert Rock
            else shape.setFillColor(sf::Color(200, 200, 200));
        }
        
        trailColor = shape.getFillColor();
    }
    
    /**
     * @brief Renders the body and its historical orbital trail.
     */
    void draw(sf::RenderWindow& window) {
        // RENDER TRAIL
        // Using VertexArray with LinesStrip for GPU efficiency
        if (path.size() > 1) {
            sf::VertexArray lines(sf::LinesStrip, path.size());
            
            for (size_t i = 0; i < path.size(); ++i) {
                lines[i].position = path[i];
                
                // ALPHA FADING LOGIC
                // Older positions (low index) become increasingly transparent
                sf::Color c = trailColor;
                float alphaFactor = static_cast<float>(i) / path.size();
                c.a = static_cast<sf::Uint8>(255 * alphaFactor);
                lines[i].color = c;
            }
            
            window.draw(lines);
        }
        window.draw(shape);
    }
};