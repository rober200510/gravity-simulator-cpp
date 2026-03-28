#include <SFML/Graphics.hpp>
#include "Body.hpp"
#include "Physics.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>

const float PI = 3.14159265f;

//Helper to format a float with N decimal places
static std::string fmt(float value, int decimals = 2) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(decimals) << value;
    return ss.str();
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
    
    // --- CAMERA SYSTEM CONFIGURATION ---
    sf::View view = window.getDefaultView();
    bool isDragging = false;
    sf::Vector2i oldMousePos;

    // --- VARIABLE TIME-STEP SYSTEM (TIME-WARP) ---
    // Decouples simulation time from real-time to allow fast-forwarding/slow-motion
    float timeScale = 1.0f;
    const float frameBaseDt = 1.0f / 60.0f;
    const float maxSafeSubDt = frameBaseDt / 4.0f;

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
    bodies.back().name = "Sol";
    
    // Define the planetary system configuration
    struct PlanetConfig {
        float dist;
        float mass;
        sf::Color color;
        std::string name;
    };
    
    std::vector<PlanetConfig> planets = {
        {120.0f, 1.5f, sf::Color(169, 169, 169), "Mercury"},  
        {180.0f, 4.0f, sf::Color(255, 140, 0), "Venus"},
        {260.0f, 5.0f, sf::Color(0, 100, 255), "Earth"},
        {340.0f, 2.5f, sf::Color(255, 50, 50), "Mars"},
        {650.0f, 12.0f, sf::Color(210, 180, 140), "Jupiter"},
        {1100.0f, 10.0f, sf::Color(238, 232, 170), "Saturn"},
        {2000.0f, 7.0f, sf::Color(173, 216, 230), "Uranus"},
        {3000.0f, 8.0f, sf::Color(0, 0, 128), "Neptune"},
        {3900.0f, 0.8f, sf::Color(200, 180, 180), "Pluto"}
    };
    
    // Instantiate planets and calculate perfect circular orbits
    for (auto& p : planets) {
        float px = centerX + p.dist;
        float py = centerY;
        
        Body planet(px, py, p.mass, p.color);
        
        // Calculate orbital velocity: v = sqrt(G * M / r)
        float velocity = std::sqrt((G * sunMass) / p.dist);
        planet.velocity.y = velocity; // Initial push perpendicular to the Sun
        
        planet.name = p.name;

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
        
        asteroid.name        = "Asteroid " + std::to_string(i + 1);

        bodies.push_back(asteroid);
    }
    
    // Physics time-stepping configuration
    const float dt = 1.0f / 60.0f;
    const int substeps = 4; 
    const float subdt = dt / substeps;
    
    // Store initial energy for potential diagnostic/debugging use
    float initialEnergy = computeTotalEnergy(bodies, G);

    //TELEMETRY: Font + selection state
    sf::Font font;
    bool fontLoaded = font.loadFromFile("C:/Windows/Fonts/consola.ttf");
    if (!fontLoaded) {
        // Fallback
        fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    int selectedBody = -1;

    //Highlight selected body
    sf::CircleShape selectionRing;
    selectionRing.setFillColor(sf::Color::Transparent);
    selectionRing.setOutlineColor(sf::Color(255, 255, 0, 200));
    selectionRing.setOutlineThickness(2.0f);

    // Main simulation loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // --- SCROLL-BASED ZOOM ---
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    // Determine zoom direction: delta > 0 (Zoom In), delta < 0 (Zoom Out)
                    float zoomFactor = (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                    view.zoom(zoomFactor);
                }
            }
            
            // --- PANNING CONTROL (Input Detection) ---
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = true;
                    // Capture initial mouse position in pixel coordinates
                    oldMousePos = sf::Mouse::getPosition(window);
                }
                if (event.mouseButton.button == sf::Mouse::Right) {
                    sf::Vector2i pixelPos(event.mouseButton.x, event.mouseButton.y);
                    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, view);

                    selectedBody       = -1;
                    float bestDist     = 1e9f;

                    for (size_t i = 0; i < bodies.size(); i++) {
                        sf::Vector2f diff = worldPos - bodies[i].position;
                        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                        // Área de selección = radio visual * 2 (mínimo 20 unidades)
                        float selRadius = std::max(bodies[i].shape.getRadius() * 2.0f, 20.0f);

                        if (dist < selRadius && dist < bestDist) {
                            bestDist     = dist;
                            selectedBody = static_cast<int>(i);
                        }
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = false;
                }
            }
            
            /// --- CAMERA TRANSLATION (Panning Logic) ---
            if (event.type == sf::Event::MouseMoved) {
                if (isDragging) {
                    sf::Vector2i newMousePos = sf::Mouse::getPosition(window);
                    
                    // Transform pixel-space coordinates into world-space coordinates
                    // This ensures consistent movement speed regardless of current zoom level
                    sf::Vector2f oldWorldPos = window.mapPixelToCoords(oldMousePos, view);
                    sf::Vector2f newWorldPos = window.mapPixelToCoords(newMousePos, view);
                    
                    // Calculate the spatial displacement vector
                    sf::Vector2f delta = oldWorldPos - newWorldPos;
                    view.move(delta);
                    
                    // Update reference position for the next frame
                    oldMousePos = newMousePos; 
                }
            }

            // --- TIME-WARP CONTROLS ---
            if (event.type == sf::Event::KeyPressed) {
                // Increase simulation speed (Fast-forward)
                if (event.key.code == sf::Keyboard::Right) {
                    timeScale *= 2.0f;
                    // Cap the maximum time scale to prevent CPU bottlenecking and frame drops
                    if (timeScale > 1024.0f) timeScale = 1024.0f; 
                    std::cout << "Velocidad de tiempo: " << timeScale << "x\n";
                }
                // Decrease simulation speed (Slow-motion)
                if (event.key.code == sf::Keyboard::Left) {
                    timeScale /= 2.0f;
                    if (timeScale < 0.125f) timeScale = 0.125f; // Limit
                    std::cout << "Velocidad de tiempo: " << timeScale << "x\n";
                }
                // Reset simulation speed to standard real-time
                if (event.key.code == sf::Keyboard::Slash || event.key.code == sf::Keyboard::Space) {
                    timeScale = 1.0f;
                    std::cout << "Velocidad restaurada a 1x\n";
                }
            }

            //TELEMETRY: Escape key clears selection
            if (event.key.code == sf::Keyboard::Escape) {
                selectedBody = -1;
            }
        }
        
        // --- DYNAMIC SUB-STEPPING CALCULATION ---
        // Determine the total simulation time to advance during the current frame
        float currentFrameDt = frameBaseDt * timeScale;
        
        // Calculate the minimum number of substeps required to keep the delta time strictly below maxSafeSubDt
        int currentSubsteps = std::max(1, static_cast<int>(std::ceil(currentFrameDt / maxSafeSubDt)));
        
        // Distribute the frame's delta time evenly across the calculated substeps
        float currentSubDt = currentFrameDt / currentSubsteps;

        // Execute the RK4 integration using the dynamically scaled substeps
        for (int sub = 0; sub < currentSubsteps; sub++) {
            integrateRK4(bodies, currentSubDt, G);
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

        window.setView(view);
        
        for (auto& body : bodies) {
            body.draw(window);
        }

        //TELEMETRY: Selection ring in world space
        if (selectedBody >= 0 && selectedBody < static_cast<int>(bodies.size())) {
            const Body& sel = bodies[selectedBody];
            float r = sel.shape.getRadius() + 5.0f;
            selectionRing.setRadius(r);
            selectionRing.setOrigin(r, r);
            selectionRing.setPosition(sel.position);
            window.draw(selectionRing);
        }

        //TELEMETRY: HUD in screen space
        // Switch to default view so the panel
        // doesn't move with the camera or scale with zoom
        window.setView(window.getDefaultView());

        if (fontLoaded) {
            //INSTRUCTIONS (bottom-left corner)
            sf::Text hint;
            hint.setFont(font);
            hint.setCharacterSize(13);
            hint.setFillColor(sf::Color(160, 160, 160));
            hint.setString("Clic derecho: seleccionar cuerpo  |  Scroll: zoom  |  "
                           "Arrastrar: paneo  |  Flechas: velocidad  |  Esc: deseleccionar");
            hint.setPosition(10.0f, 900.0f - 25.0f);
            window.draw(hint);

            //TELEMETRY PANEL
            if (selectedBody >= 0 && selectedBody < static_cast<int>(bodies.size())) {
                const Body& sel = bodies[selectedBody];

                float vx    = sel.velocity.x;
                float vy    = sel.velocity.y;
                float speed = std::sqrt(vx * vx + vy * vy);

                // Build data string
                std::string data =
                    "[ " + sel.name + " ]\n"
                    "\n"
                    "Pos  X  :  " + fmt(sel.position.x) + "\n"
                    "Pos  Y  :  " + fmt(sel.position.y) + "\n"
                    "\n"
                    "Vel  X  :  " + fmt(vx, 3) + "\n"
                    "Vel  Y  :  " + fmt(vy, 3) + "\n"
                    "\n"
                    "Speed   :  " + fmt(speed, 3) + " u/s\n"
                    "\n"
                    "Masa    :  " + fmt(sel.mass, 2);

                sf::Text telText;
                telText.setFont(font);
                telText.setString(data);
                telText.setCharacterSize(15);
                telText.setFillColor(sf::Color(220, 220, 220));
                telText.setPosition(20.0f, 20.0f);

                // Measure text size for the background panel
                sf::FloatRect bounds = telText.getLocalBounds();
                float padX = 16.0f, padY = 12.0f;

                //Semi-transparent background
                sf::RectangleShape panel(sf::Vector2f(bounds.width  + padX * 2,
                                                      bounds.height + padY * 2));
                panel.setPosition(10.0f, 10.0f);
                panel.setFillColor(sf::Color(10, 10, 20, 210));
                panel.setOutlineColor(sel.trailColor);
                panel.setOutlineThickness(1.5f);
                window.draw(panel);

                //Header
                sf::Text header;
                header.setFont(font);
                header.setString("[ " + sel.name + " ]");
                header.setCharacterSize(16);
                header.setFillColor(sel.trailColor);
                header.setStyle(sf::Text::Bold);
                header.setPosition(20.0f, 22.0f);

                // Replace name line with text excluding that line
                std::string dataNoName =
                    "\n"
                    "\n"
                    "Pos  X  :  " + fmt(sel.position.x) + "\n"
                    "Pos  Y  :  " + fmt(sel.position.y) + "\n"
                    "\n"
                    "Vel  X  :  " + fmt(vx, 3) + "\n"
                    "Vel  Y  :  " + fmt(vy, 3) + "\n"
                    "\n"
                    "Speed   :  " + fmt(speed, 3) + " u/s\n"
                    "\n"
                    "Masa    :  " + fmt(sel.mass, 2);

                telText.setString(dataNoName);

                window.draw(panel);
                window.draw(header);
                window.draw(telText);
            }
        }
        
        window.display();
    }
    
    return 0;
}