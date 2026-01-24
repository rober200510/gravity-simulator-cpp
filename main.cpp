#include <SFML/Graphics.hpp>
#include "Body.hpp"
#include <vector>
#include <cmath>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Gravity Simulator");
    window.setFramerateLimit(60);

    // --- INITIALIZATION ---
    std::vector<Body> bodies;
    bodies.push_back(Body(400, 300, 100)); //sun-like

    bodies.push_back(Body(200, 300, 10)); //planet
    bodies[1].velocity.y = 15.0f;

    bodies.push_back(Body(600, 300, 5)); //planet
    bodies[2].velocity.y = -15.0f;

    const float G = 500.0f; //As an example

    sf::Clock clock; //For dt

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        //update logic
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();

        //N-Body gravity algorithm
        for (size_t i = 0; i < bodies.size(); i++){
            for (size_t j = 0; j < bodies.size(); j++){
                if (i == j) continue; // Do not calculate gravity against self

                Body& current = bodies[i];
                Body& other = bodies[j];

                sf::Vector2f direction = other.position - current.position;
                float distanceSq = direction.x * direction.x + direction.y * direction.y;
                float distance = std::sqrt(distanceSq);

                
                if (distance < 50.0f) continue; // Prevent infinite forces when bodies are too close or overlapping

                //Newton's Law of Universal Gravitation
                float forceMagnitude = G * current.mass * other.mass / distanceSq;

                // Normalize direction vector and scale by force magnitude
                sf::Vector2f forceVec = (direction / distance) * forceMagnitude;

                // Add acceleration to velocity (a = F/m)
                current.velocity += (forceVec / current.mass) * dt;
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