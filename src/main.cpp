#include "GameWindow.hpp"
#include "Level.hpp"
#include "Physics.hpp"
#include "Player.hpp"
#include <iostream>
#include <memory>
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
  if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
    std::cerr << "Failed to load font C:/Windows/Fonts/arial.ttf" << std::endl;
    // Proceeding without font might crash or show nothing if we use text.
    // SFML requires a valid font for Text.
  }

  // SFML 3: Text constructor requires font as first argument
  sf::Text uiText(font);
  uiText.setCharacterSize(40);
  uiText.setFillColor(sf::Color::White);

  sf::Text hudText(font);
  hudText.setCharacterSize(30);
  hudText.setFillColor(sf::Color::White);
  hudText.setPosition({20.f, 20.f}); // SFML 3: functions take sf::Vector2f

  // Game State
  int lives = 3;
  enum State { PLAYING, DEATH_ANIM, LIVES_SCREEN, GAME_OVER };
  State currentState = PLAYING;
  float stateTimer = 0.0f;

  // Session
  std::unique_ptr<GameSession> session =
      std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT);

  // Camera
  // SFML 3: Rect constructor takes vectors: position, size
  sf::View camera(sf::FloatRect({0.f, 0.f}, {(float)WIDTH, (float)HEIGHT}));

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
      // Block right movement (maxCamX) - usar el ancho del nivel
      float maxCamX = session->level->getLevelWidth() - (float)WIDTH / 2.0f;
      float camX = std::max(session->player->getPosition().x, minCamX);
      camX = std::min(camX, maxCamX); // Limitar al borde derecho

      // Block down movement (maxCamY) - Ground is at bottom
      float maxCamY = (float)HEIGHT / 2.0f;
      float camY = std::min(session->player->getPosition().y, maxCamY);

      camera.setCenter({camX, camY}); // SFML 3: takes Vector2f

      // Check Death
      if (session->player->isDead()) {
        currentState = DEATH_ANIM;
      } else if (session->player->getPosition().y > (float)HEIGHT + 50.0f) {
        // Fell into void
        session->player->die(); // Instant death even if big
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
        camera.setCenter({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});
      }
    } else if (currentState == GAME_OVER) {
      stateTimer -= dt;
      if (stateTimer <= 0.0f) {
        // Restart Game
        lives = 3;
        session = std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT);
        currentState = LIVES_SCREEN;
        stateTimer = 2.0f;
        camera.setCenter({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});
      }
    }
  };

  auto render = [&]() {
    if (currentState == PLAYING || currentState == DEATH_ANIM) {
      window.window().setView(camera);
      session->level->draw(window.window());
      session->player->draw(window.window());

      // Draw HUD
      window.window().setView(window.window().getDefaultView());
      hudText.setString("Lives: " + std::to_string(lives));
      window.window().draw(hudText);
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
      // SFML 3: Rect members are position.x/y and size.x/y
      uiText.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                        textBounds.position.y + textBounds.size.y / 2.0f});
      uiText.setPosition({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});

      window.window().draw(uiText);
    }
  };

  window.run(update, render);

  return 0;
}
