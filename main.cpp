#include <SFML/Graphics.hpp>
#include "Body.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Mathematical constant for PI
const float PI = 3.14159265f;

int main() {
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Gravity Simulator");
    window.setFramerateLimit(60);

    // Seed the random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // --- CONFIGURATION ---
    const float G = 100.0f;
    std::vector<Body> bodies;

    // 1. The Sun
    float centerX = 1600.0f / 2.0f;
    float centerY = 900.0f / 2.0f;
    float sunMass = 10000.0f; 
    
    bodies.push_back(Body(centerX, centerY, sunMass, sf::Color::Yellow));


    // 2. Planets
    // Simple structure to configure planets quickly
    struct PlanetConfig {
        float dist;     // Distance from sun
        float mass;
        sf::Color color;
    };

    std::vector<PlanetConfig> planets = {
        {120.0f, 20.0f, sf::Color(169, 169, 169)}, // Mercury
        {180.0f, 40.0f, sf::Color(255, 140, 0)},   // Venus
        {260.0f, 50.0f, sf::Color(0, 100, 255)},   // Earth
        {340.0f, 35.0f, sf::Color(255, 50, 50)},   // Mars
        {650.0f, 300.0f, sf::Color(210, 180, 140)} // Jupiter
    };

    for (auto& p : planets) {
        // Initial Position: Right side of the sun (Alignment)
        float px = centerX + p.dist;
        float py = centerY;

        // Create the body with specific color
        Body planet(px, py, p.mass, p.color);

        // Orbital Velocity: v = sqrt(G * M / r)
        // Since they are on the right (X axis), velocity is down (positive Y)
        float velocity = std::sqrt((G * sunMass) / p.dist);
        planet.velocity.y = velocity; 

        bodies.push_back(planet);
    }

    // Asteroid generation
    int numAsteroids = 10;

    for (int i = 0; i < numAsteroids; i++) {
        float angle = static_cast<float>(std::rand() % 360) * PI / 180.0f;
        
        // Belt between 400 and 530 (Far enough from Mars, inside Jupiter)
        float dist = 400.0f + static_cast<float>(std::rand() % 130);

        float rx = centerX + std::cos(angle) * dist;
        float ry = centerY + std::sin(angle) * dist;

        // Small masses
        float rm = static_cast<float>(std::rand() % 5 + 1); 

        Body asteroid(rx, ry, rm);

        // Circular orbit calculation
        float orbitalSpeed = std::sqrt((G * sunMass) / dist);
        asteroid.velocity.x = -std::sin(angle) * orbitalSpeed;
        asteroid.velocity.y = std::cos(angle) * orbitalSpeed;

        bodies.push_back(asteroid);
    }

    sf::Clock clock; //For dt

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        //update logic
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        for (auto& body : bodies) {
            body.acceleration = {0.0f, 0.0f};
        }

        //N-Body gravity algorithm
        for (size_t i = 0; i < bodies.size(); i++){

            // Optimization: Skip force calculation FOR the Sun (Index 0).
            // This anchors the sun in the center, acting as a static gravity well.
            if (i == 0) continue;

            for (size_t j = 0; j < bodies.size(); j++){
                if (i == j) continue; // Do not calculate gravity against self

                Body& current = bodies[i];
                Body& other = bodies[j];

                sf::Vector2f direction = other.position - current.position;
                float distanceSq = direction.x * direction.x + direction.y * direction.y;
                float distance = std::sqrt(distanceSq);

                
                if (distance < 20.0f) continue; // Prevent infinite forces when bodies are too close or overlapping

                //Newton's Law of Universal Gravitation
                float forceMagnitude = G * current.mass * other.mass / distanceSq;

                // Normalize direction vector and scale by force magnitude
                sf::Vector2f forceVec = (direction / distance) * forceMagnitude;

                // Add acceleration to velocity (a = F/m)
                current.acceleration += (forceVec / current.mass);
            }
        }

        for (auto& body : bodies) {
            body.update(dt);
        }

        // --- DRAW ---
        window.clear(sf::Color::Black);

        for (auto& body : bodies) {
            body.draw(window);
        }
        window.display();
    }
    return 0;
}