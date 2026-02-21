#include "Physics.hpp"
#include <cmath>

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