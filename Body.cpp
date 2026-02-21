#include "Body.hpp"

Body::Body(float x, float y, float m, sf::Color c) {
    position.x = x;
    position.y = y;
    mass = m;
    velocity = {0.0f, 0.0f};
    
    // VISUAL RADIUS LOGIC
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
    shape.setOrigin(visualRadius, visualRadius); 
    shape.setPosition(position);
    
    // AUTOMATIC COLOR ASSIGNMENT
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

void Body::draw(sf::RenderWindow& window) {
    // RENDER TRAIL
    if (path.size() > 1) {
        sf::VertexArray lines(sf::LinesStrip, path.size());
        
        for (size_t i = 0; i < path.size(); ++i) {
            lines[i].position = path[i];
            
            // ALPHA FADING LOGIC
            sf::Color c = trailColor;
            float alphaFactor = static_cast<float>(i) / path.size();
            c.a = static_cast<sf::Uint8>(255 * alphaFactor);
            lines[i].color = c;
        }
        
        window.draw(lines);
    }
    window.draw(shape);
}