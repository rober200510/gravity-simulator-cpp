#pragma once
#include <SFML/Graphics.hpp>
#include <deque>

class Body {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float mass;
    
    sf::CircleShape shape;
    
    std::deque<sf::Vector2f> path;
    size_t maxPathLength = 5000;
    sf::Color trailColor;
    
    Body(float x, float y, float m, sf::Color c = sf::Color::Transparent) {
        position.x = x;
        position.y = y;
        mass = m;
        velocity = {0.0f, 0.0f};
        
        float visualRadius;
        if (m >= 1000.0f) {
            visualRadius = 45.0f;
        } else if (m >= 10.0f) {
            visualRadius = m / 2.5f;
        } else {
            visualRadius = m * 0.6f;
            if (visualRadius < 1.5f) visualRadius = 1.5f;
        }
        
        shape.setRadius(visualRadius);
        shape.setOrigin(visualRadius, visualRadius);
        shape.setPosition(position);
        
        if (c != sf::Color::Transparent) {
            shape.setFillColor(c);
        } else {
            if (m >= 15000.0f) shape.setFillColor(sf::Color(100, 150, 255));
            else if (m >= 3000.0f) shape.setFillColor(sf::Color::White);
            else if (m >= 1000.0f) shape.setFillColor(sf::Color::Yellow);
            else if (m >= 200.0f) shape.setFillColor(sf::Color(255, 100, 50));
            else shape.setFillColor(sf::Color(200, 200, 200));
        }
        
        trailColor = shape.getFillColor();
    }
    
    void draw(sf::RenderWindow& window) {
        if (path.size() > 1) {
            sf::VertexArray lines(sf::LinesStrip, path.size());
            
            for (size_t i = 0; i < path.size(); ++i) {
                lines[i].position = path[i];
                
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