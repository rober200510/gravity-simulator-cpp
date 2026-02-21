#include <SFML/Graphics.hpp>
#include "Body.hpp"
#include "Physics.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

const float PI = 3.14159265f;

/**
 * @brief Main entry point of the application.
 * Initializes the window, generates the celestial bodies (Sun, planets, asteroid belt),
 * and runs the physics simulation loop using sub-stepping.
 */
int main() {
    // Initialize SFML RenderWindow with fixed 60 FPS
    sf::RenderWindow window(sf::VideoMode(1600, 900), "N-Body Gravity Simulator - RK4");
    window.setFramerateLimit(60);
    
    // Seed the random number generator for the asteroid belt
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    // Simulation constants
    const float G = 100.0f;
    std::vector<Body> bodies;
    
    // Create the central star (Sun) and add it to the system
    float centerX = 800.0f;
    float centerY = 450.0f;
    float sunMass = 10000.0f;
    
    bodies.push_back(Body(centerX, centerY, sunMass, sf::Color::Yellow));
    
    // Define the planetary system configuration
    struct PlanetConfig {
        float dist;
        float mass;
        sf::Color color;
    };
    
    std::vector<PlanetConfig> planets = {
        {120.0f, 1.5f, sf::Color(169, 169, 169)},  
        {180.0f, 4.0f, sf::Color(255, 140, 0)},
        {260.0f, 5.0f, sf::Color(0, 100, 255)},
        {340.0f, 2.5f, sf::Color(255, 50, 50)},
        {650.0f, 12.0f, sf::Color(210, 180, 140)}
    };
    
    // Instantiate planets and calculate perfect circular orbits
    for (auto& p : planets) {
        float px = centerX + p.dist;
        float py = centerY;
        
        Body planet(px, py, p.mass, p.color);
        
        // Calculate orbital velocity: v = sqrt(G * M / r)
        float velocity = std::sqrt((G * sunMass) / p.dist);
        planet.velocity.y = velocity; // Initial push perpendicular to the Sun
        
        bodies.push_back(planet);
    }
    
    // Generate a random asteroid belt
    int numAsteroids = 10;
    for (int i = 0; i < numAsteroids; i++) {
        // Randomize angle (radians) and distance
        float angle = static_cast<float>(std::rand() % 360) * PI / 180.0f;
        float dist = 400.0f + static_cast<float>(std::rand() % 130);
        
        // Convert polar to Cartesian coordinates
        float rx = centerX + std::cos(angle) * dist;
        float ry = centerY + std::sin(angle) * dist;
        float rm = 0.1f + static_cast<float>(std::rand() % 10) * 0.05f; // 0.1 - 0.6
        
        Body asteroid(rx, ry, rm);
        
        // Calculate orbital velocity vector based on angle
        float orbitalSpeed = std::sqrt((G * sunMass) / dist);
        asteroid.velocity.x = -std::sin(angle) * orbitalSpeed;
        asteroid.velocity.y = std::cos(angle) * orbitalSpeed;
        
        bodies.push_back(asteroid);
    }
    
    // Physics time-stepping configuration
    const float dt = 1.0f / 60.0f;
    const int substeps = 4; 
    const float subdt = dt / substeps;
    
    // Store initial energy for potential diagnostic/debugging use
    float initialEnergy = computeTotalEnergy(bodies, G);
    
    // Main simulation loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }
        
        // Physics Sub-stepping: Perform multiple updates per frame for higher accuracy
        for (int sub = 0; sub < substeps; sub++) {
            integrateRK4(bodies, subdt, G);
        }
        
        // Update visual paths (trails) and graphical shapes
        for (auto& body : bodies) {
            body.path.push_back(body.position);
            if (body.path.size() > body.maxPathLength) {
                body.path.pop_front();
            }
            body.shape.setPosition(body.position);
        }
        
        // Render Frame
        window.clear(sf::Color::Black);
        
        for (auto& body : bodies) {
            body.draw(window);
        }
        
        window.display();
    }
    
    return 0;
}