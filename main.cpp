#include <SFML/Graphics.hpp>
#include "Body.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

const float PI = 3.14159265f;

/**
 * @brief System state snapshot used for RK4 multi-step predictions
 */

struct State {
    std::vector<sf::Vector2f> positions;
    std::vector<sf::Vector2f> velocities;
};

/**
 * @brief Computes net gravitational acceleration for all bodies
 * Uses Newton's Law of Universal Gravitation with Softening
 */
std::vector<sf::Vector2f> computeAccelerations(const std::vector<Body>& bodies, 
                                                const std::vector<sf::Vector2f>& positions,
                                                float G) {
    std::vector<sf::Vector2f> accelerations(bodies.size(), sf::Vector2f(0.0f, 0.0f));
    
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = 0; j < bodies.size(); j++) {
            // Optimization: Anchor the Sun (index 0) to the center
            if (i == j) continue;
            
            sf::Vector2f direction = positions[j] - positions[i];
            float distanceSq = direction.x * direction.x + direction.y * direction.y;

            // GRAVITATIONAL SOFTENING
            // Prevents division by zero and infinite forces during close encounters
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

/**
 * @brief Runge-Kutta 4th Order Integrator.
 * Samples the derivative at 4 points to achieve O(dt^4) precision.
 */

void integrateRK4(std::vector<Body>& bodies, float dt, float G) {
    size_t n = bodies.size();
    
    State s0; // Current State
    s0.positions.resize(n);
    s0.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s0.positions[i] = bodies[i].position;
        s0.velocities[i] = bodies[i].velocity;
    }
    
    // k1: Derivatives at the start of the interval
    auto k1_acc = computeAccelerations(bodies, s0.positions, G);
    std::vector<sf::Vector2f> k1_vel = s0.velocities;
    
    State s1;
    s1.positions.resize(n);
    s1.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s1.positions[i] = s0.positions[i] + k1_vel[i] * (dt * 0.5f);
        s1.velocities[i] = s0.velocities[i] + k1_acc[i] * (dt * 0.5f);
    }
    
    // k2: Midpoint prediction using k1
    auto k2_acc = computeAccelerations(bodies, s1.positions, G);
    std::vector<sf::Vector2f> k2_vel = s1.velocities;
    
    State s2;
    s2.positions.resize(n);
    s2.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s2.positions[i] = s0.positions[i] + k2_vel[i] * (dt * 0.5f);
        s2.velocities[i] = s0.velocities[i] + k2_acc[i] * (dt * 0.5f);
    }
    
    // k3: Midpoint prediction using k2
    auto k3_acc = computeAccelerations(bodies, s2.positions, G);
    std::vector<sf::Vector2f> k3_vel = s2.velocities;
    
    State s3;
    s3.positions.resize(n);
    s3.velocities.resize(n);
    for (size_t i = 0; i < n; i++) {
        s3.positions[i] = s0.positions[i] + k3_vel[i] * dt;
        s3.velocities[i] = s0.velocities[i] + k3_acc[i] * dt;
    }
    
    // k4: End-point prediction using k3
    auto k4_acc = computeAccelerations(bodies, s3.positions, G);
    std::vector<sf::Vector2f> k4_vel = s3.velocities;
    
    // FINAL INTEGRATION
    // Weighted average: (k1 + 2k2 + 2k3 + k4) / 6
    for (size_t i = 0; i < n; i++) {
        bodies[i].position = s0.positions[i] + (k1_vel[i] + k2_vel[i] * 2.0f + k3_vel[i] * 2.0f + k4_vel[i]) * (dt / 6.0f);
        bodies[i].velocity = s0.velocities[i] + (k1_acc[i] + k2_acc[i] * 2.0f + k3_acc[i] * 2.0f + k4_acc[i]) * (dt / 6.0f);
    }
}

/**
 * @brief Calculates the total mechanical energy (Kinetic + Potential) of the system.
 * Used to verify the stability and accuracy of the RK4 integration over time.
 * @param bodies Vector of all celestial bodies in the simulation.
 * @param G The gravitational constant.
 * @return Total system energy as a float.
 */
float computeTotalEnergy(const std::vector<Body>& bodies, float G) {
    float kinetic = 0.0f;
    float potential = 0.0f;
    
    // Calculate Kinetic Energy (Ek = 1/2 * m * v^2) for all bodies
    for (const auto& body : bodies) {
        float v2 = body.velocity.x * body.velocity.x + body.velocity.y * body.velocity.y;
        kinetic += 0.5f * body.mass * v2;
    }
    
    // Calculate Potential Energy (Ep = -G * m1 * m2 / r)
    // Uses unique pairs (j = i + 1) to avoid double counting
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