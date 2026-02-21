#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "Body.hpp"

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
std::vector<sf::Vector2f> computeAccelerations(const std::vector<Body>& bodies, const std::vector<sf::Vector2f>& positions, float G);

/**
 * @brief Runge-Kutta 4th Order Integrator.
 * Samples the derivative at 4 points to achieve O(dt^4) precision.
 */
void integrateRK4(std::vector<Body>& bodies, float dt, float G);

/**
 * @brief Calculates the total mechanical energy (Kinetic + Potential) of the system.
 * Used to verify the stability and accuracy of the RK4 integration over time.
 */
float computeTotalEnergy(const std::vector<Body>& bodies, float G);