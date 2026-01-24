#include <SFML/Graphics.hpp>
#include "Body.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Gravity Simulator");
    window.setFramerateLimit(60);

    // --- INITIALIZATION ---
    Body planet(400.0f, 300.0f, 20.0f); 

    // Give it an initial push to the right (50 pixels per second)
    planet.velocity.x = 50.0f; 

    sf::Clock clock; //For dt

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        //update logic
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();
        planet.update(dt);

        // --- DRAW ---
        window.clear(sf::Color::Black);
        planet.draw(window);
        window.display();
    }
    return 0;
}