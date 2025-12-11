#include "GameWindow.hpp"
#include "Physics.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include <memory>
#include <iostream>
#include <string>

// Encapsulate Game Session to easily reset level
struct GameSession {
    Physics physics;
    std::unique_ptr<Level> level;
    std::unique_ptr<Player> player;

    GameSession(float width, float height) {
        level = std::make_unique<Level>(physics, width, height);
        player = std::make_unique<Player>(physics, 100.0f, 400.0f);
    }
};

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;

    GameWindow window(WIDTH, HEIGHT, "Mario - Demo (SFML + Box2D)");

    // Load Font (Fallback system font since project might miss one)
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Failed to load font C:/Windows/Fonts/arial.ttf" << std::endl;
        // Proceeding without font might crash or show nothing if we use text.
        // SFML requires a valid font for Text.
    }
    
    sf::Text uiText;
    uiText.setFont(font);
    uiText.setCharacterSize(40);
    uiText.setFillColor(sf::Color::White);

    // Game State
    int lives = 3;
    enum State { PLAYING, DEATH_ANIM, LIVES_SCREEN, GAME_OVER };
    State currentState = PLAYING;
    float stateTimer = 0.0f;

    // Session
    std::unique_ptr<GameSession> session = std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT);

    // Camera
    sf::View camera(sf::FloatRect(0, 0, (float)WIDTH, (float)HEIGHT));

    auto update = [&](float dt) {
        if (currentState == PLAYING) {
            session->physics.step(dt);
            session->player->handleInput(dt);
            session->player->update(dt);
            session->level->update(dt);
            session->level->checkCollisions(*session->player);

            // Camera Follow with Constraints
            // Block left movement (minCamX)
            float minCamX = (float)WIDTH / 2.0f;
            float camX = std::max(session->player->getPosition().x, minCamX);
            
            // Block down movement (maxCamY) - Ground is at bottom
            float maxCamY = (float)HEIGHT / 2.0f;
            float camY = std::min(session->player->getPosition().y, maxCamY);
            
            camera.setCenter(camX, camY);

            // Check Death
            if (session->player->isDead()) {
                currentState = DEATH_ANIM;
            }

        } else if (currentState == DEATH_ANIM) {
             // Continue physics for destruction/falling
             session->physics.step(dt); 
             session->player->update(dt); 

             // Check Below Map
             // Ground is ~568. 
             if (session->player->getPosition().y > (float)HEIGHT + 100.0f) {
                 lives--;
                 if (lives > 0) {
                     currentState = LIVES_SCREEN;
                     stateTimer = 2.0f;
                 } else {
                     currentState = GAME_OVER;
                     stateTimer = 3.0f;
                 }
             }
        } else if (currentState == LIVES_SCREEN) {
             stateTimer -= dt;
             if (stateTimer <= 0.0f) {
                 // Reset Level
                 session = std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT);
                 currentState = PLAYING;
                 camera.setCenter((float)WIDTH / 2.0f, (float)HEIGHT / 2.0f);
             }
        } else if (currentState == GAME_OVER) {
             stateTimer -= dt;
             if (stateTimer <= 0.0f) {
                 // Restart Game
                 lives = 3;
                 session = std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT);
                 currentState = LIVES_SCREEN; 
                 stateTimer = 2.0f;
                 camera.setCenter((float)WIDTH / 2.0f, (float)HEIGHT / 2.0f);
             }
        }
    };

    auto render = [&]() {
        if (currentState == PLAYING || currentState == DEATH_ANIM) {
            window.window().setView(camera);
            session->level->draw(window.window());
            session->player->draw(window.window());
        } else {
            // Draw Black Screen with UI
            window.window().setView(window.window().getDefaultView());
            
            sf::RectangleShape blackScreen(sf::Vector2f((float)WIDTH, (float)HEIGHT));
            blackScreen.setFillColor(sf::Color::Black);
            window.window().draw(blackScreen);
            
            if (currentState == LIVES_SCREEN) {
                uiText.setString(std::to_string(lives) + " Lives");
            } else {
                uiText.setString("GAME OVER");
            }
            
            // Center Text
            sf::FloatRect textBounds = uiText.getLocalBounds();
            uiText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                             textBounds.top + textBounds.height / 2.0f);
            uiText.setPosition((float)WIDTH / 2.0f, (float)HEIGHT / 2.0f);
            
            window.window().draw(uiText);
        }
    };

    window.run(update, render);

    return 0;
}
