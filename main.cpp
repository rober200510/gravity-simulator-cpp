#include <SFML/Graphics.hpp>
#include "Body.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

const float PI = 3.14159265f;

struct State {
    std::vector<sf::Vector2f> positions;
    std::vector<sf::Vector2f> velocities;
};

std::vector<sf::Vector2f> computeAccelerations(const std::vector<Body>& bodies, 
                                                const std::vector<sf::Vector2f>& positions,
                                                float G) {
    std::vector<sf::Vector2f> accelerations(bodies.size(), sf::Vector2f(0.0f, 0.0f));
    
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = 0; j < bodies.size(); j++) {
            if (i == j) continue;
            
            sf::Vector2f direction = positions[j] - positions[i];
            float distanceSq = direction.x * direction.x + direction.y * direction.y;
            
            const float softening = 1.0f;
            distanceSq += softening * softening;
            
            float distance = std::sqrt(distanceSq);
            
            if (distance > 0.01f) {
                float forceMagnitude = G * bodies[j].mass / distanceSq;
                accelerations[i] += (direction / distance) * forceMagnitude;
            }
        }
    }
    
    return accelerations;
}

void integrateRK4(std::vector<Body>& bodies, float dt, float G) {
    size_t n = bodies.size();
    
    State s0;
    s0.positions.resize(n);
    s0.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s0.positions[i] = bodies[i].position;
        s0.velocities[i] = bodies[i].velocity;
    }
    
    auto k1_acc = computeAccelerations(bodies, s0.positions, G);
    std::vector<sf::Vector2f> k1_vel = s0.velocities;
    
    State s1;
    s1.positions.resize(n);
    s1.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s1.positions[i] = s0.positions[i] + k1_vel[i] * (dt * 0.5f);
        s1.velocities[i] = s0.velocities[i] + k1_acc[i] * (dt * 0.5f);
    }
    
    auto k2_acc = computeAccelerations(bodies, s1.positions, G);
    std::vector<sf::Vector2f> k2_vel = s1.velocities;
    
    State s2;
    s2.positions.resize(n);
    s2.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s2.positions[i] = s0.positions[i] + k2_vel[i] * (dt * 0.5f);
        s2.velocities[i] = s0.velocities[i] + k2_acc[i] * (dt * 0.5f);
    }
    
    auto k3_acc = computeAccelerations(bodies, s2.positions, G);
    std::vector<sf::Vector2f> k3_vel = s2.velocities;
    
    State s3;
    s3.positions.resize(n);
    s3.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s3.positions[i] = s0.positions[i] + k3_vel[i] * dt;
        s3.velocities[i] = s0.velocities[i] + k3_acc[i] * dt;
    }
    
    auto k4_acc = computeAccelerations(bodies, s3.positions, G);
    std::vector<sf::Vector2f> k4_vel = s3.velocities;
    
    for (size_t i = 0; i < n; i++) {
        bodies[i].position = s0.positions[i] + (k1_vel[i] + k2_vel[i] * 2.0f + k3_vel[i] * 2.0f + k4_vel[i]) * (dt / 6.0f);
        bodies[i].velocity = s0.velocities[i] + (k1_acc[i] + k2_acc[i] * 2.0f + k3_acc[i] * 2.0f + k4_acc[i]) * (dt / 6.0f);
    }
}

float computeTotalEnergy(const std::vector<Body>& bodies, float G) {
    float kinetic = 0.0f;
    float potential = 0.0f;
    
    for (const auto& body : bodies) {
        float v2 = body.velocity.x * body.velocity.x + body.velocity.y * body.velocity.y;
        kinetic += 0.5f * body.mass * v2;
    }
    
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = i + 1; j < bodies.size(); j++) {
            sf::Vector2f diff = bodies[j].position - bodies[i].position;
            float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            if (distance > 0.01f) {
                potential -= G * bodies[i].mass * bodies[j].mass / distance;
            }
        }
    }
    
    return kinetic + potential;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1600, 900), "N-Body Gravity Simulator - RK4");
    window.setFramerateLimit(60);
    
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    const float G = 100.0f;
    std::vector<Body> bodies;
    
    float centerX = 800.0f;
    float centerY = 450.0f;
    float sunMass = 10000.0f;
    
    bodies.push_back(Body(centerX, centerY, sunMass, sf::Color::Yellow));
    
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
    
    for (auto& p : planets) {
        float px = centerX + p.dist;
        float py = centerY;
        
        Body planet(px, py, p.mass, p.color);
        
        float velocity = std::sqrt((G * sunMass) / p.dist);
        planet.velocity.y = velocity;
        
        bodies.push_back(planet);
    }
    
    int numAsteroids = 10;
    for (int i = 0; i < numAsteroids; i++) {
        float angle = static_cast<float>(std::rand() % 360) * PI / 180.0f;
        float dist = 400.0f + static_cast<float>(std::rand() % 130);
        
        float rx = centerX + std::cos(angle) * dist;
        float ry = centerY + std::sin(angle) * dist;
        float rm = 0.1f + static_cast<float>(std::rand() % 10) * 0.05f; // 0.1 - 0.6
        
        Body asteroid(rx, ry, rm);
        
        float orbitalSpeed = std::sqrt((G * sunMass) / dist);
        asteroid.velocity.x = -std::sin(angle) * orbitalSpeed;
        asteroid.velocity.y = std::cos(angle) * orbitalSpeed;
        
        bodies.push_back(asteroid);
    }
    
    const float dt = 1.0f / 60.0f;
    const int substeps = 4; 
    const float subdt = dt / substeps;
    
    float initialEnergy = computeTotalEnergy(bodies, G);
    int frameCount = 0;
    
    sf::Font font;
    bool fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    
    sf::Text energyText;
    if (fontLoaded) {
        energyText.setFont(font);
        energyText.setCharacterSize(14);
        energyText.setFillColor(sf::Color::White);
        energyText.setPosition(10, 10);
    }
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }
        
        for (int sub = 0; sub < substeps; sub++) {
            integrateRK4(bodies, subdt, G);
        }
        
        for (auto& body : bodies) {
            body.path.push_back(body.position);
            if (body.path.size() > body.maxPathLength) {
                body.path.pop_front();
            }
            body.shape.setPosition(body.position);
        }
        
        frameCount++;
        if (frameCount % 60 == 0 && fontLoaded) {
            float currentEnergy = computeTotalEnergy(bodies, G);
            float energyError = std::abs((currentEnergy - initialEnergy) / initialEnergy) * 100.0f;
            
            energyText.setString("Energy Conservation Error: " + std::to_string(energyError) + "%\nFrames: " + std::to_string(frameCount));
        }
        
        window.clear(sf::Color::Black);
        
        for (auto& body : bodies) {
            body.draw(window);
        }
        
        if (fontLoaded) {
            window.draw(energyText);
        }
        
        window.display();
    }
    
    return 0;
}