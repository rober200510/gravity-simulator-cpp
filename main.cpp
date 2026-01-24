#include <SFML/Graphics.hpp>
#include "Body.hpp"
#include <vector>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Gravity Simulator");
    window.setFramerateLimit(60);

    // --- INITIALIZATION ---
    std::vector<Body> bodies;
    bodies.push_back(Body(400, 300, 100));

    bodies.push_back(Body(200, 300, 20));
    bodies[1].velocity.y = 50.0f;

    bodies.push_back(Body(600, 300, 20));
    bodies[2].velocity.y = -50.0f;

    sf::Clock clock; //For dt

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        //update logic
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();
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