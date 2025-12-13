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

  GameSession(float width, float height, int levelNumber = 1) {
    level = std::make_unique<Level>(physics, width, height, levelNumber);
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
  }

  // SFML 3: Text constructor requires font as first argument
  sf::Text uiText(font);
  uiText.setCharacterSize(40);
  uiText.setFillColor(sf::Color::White);

  sf::Text hudText(font);
  hudText.setCharacterSize(30);
  hudText.setFillColor(sf::Color::White);
  hudText.setPosition({60.f, 20.f}); // Moved right to make room for helmet icon

  // Load Helmet Icon for Lives display
  sf::Texture helmetTexture;
  if (!helmetTexture.loadFromFile("assets/images/casco.png")) {
      std::cerr << "Failed to load assets/images/casco.png" << std::endl;
  }
  sf::Sprite helmetSprite(helmetTexture);
  helmetSprite.setPosition({15.f, 18.f}); // Aligned with text center
  helmetSprite.setScale({0.5f, 0.5f}); // Much larger icon

  // Load Menu Texture
  sf::Texture menuTexture;
  if (!menuTexture.loadFromFile("assets/images/menu.jpg")) {
      std::cerr << "Failed to load assets/images/menu.jpg" << std::endl;
  }
  sf::Sprite menuSprite(menuTexture);
  
  // Scale and center the menu image to fit the 800x600 window
  sf::Vector2u texSize = menuTexture.getSize();
  menuSprite.setOrigin({texSize.x / 2.0f, texSize.y / 2.0f});
  menuSprite.setPosition({WIDTH / 2.0f, HEIGHT / 2.0f});
  
  // Scale to fit width and height explicitly
  menuSprite.setScale({(float)WIDTH / texSize.x, (float)HEIGHT / texSize.y});

  // Load Game Over Texture
  sf::Texture gameOverTexture;
  if (!gameOverTexture.loadFromFile("assets/images/gameover.png")) {
      std::cerr << "Failed to load assets/images/gameover.png" << std::endl;
  }
  sf::Sprite gameOverSprite(gameOverTexture);
  
  // Center the game over image
  sf::Vector2u goTexSize = gameOverTexture.getSize();
  gameOverSprite.setOrigin({goTexSize.x / 2.0f, goTexSize.y / 2.0f});
  gameOverSprite.setPosition({WIDTH / 2.0f, HEIGHT / 2.0f});
  
  // Scale to fit the screen
  gameOverSprite.setScale({(float)WIDTH / goTexSize.x, (float)HEIGHT / goTexSize.y});

  // Load Final/Victory Texture
  sf::Texture finalTexture;
  if (!finalTexture.loadFromFile("assets/images/Final.png")) {
      std::cerr << "Failed to load assets/images/Final.png" << std::endl;
  }
  sf::Sprite finalSprite(finalTexture);
  
  // Center the final image
  sf::Vector2u finalTexSize = finalTexture.getSize();
  finalSprite.setOrigin({finalTexSize.x / 2.0f, finalTexSize.y / 2.0f});
  finalSprite.setPosition({WIDTH / 2.0f, HEIGHT / 2.0f});
  
  // Scale to fit the screen
  finalSprite.setScale({(float)WIDTH / finalTexSize.x, (float)HEIGHT / finalTexSize.y});


  // Load Menu Sound
  sf::SoundBuffer menuSoundBuffer;
  if (!menuSoundBuffer.loadFromFile("assets/music/menu_sound.wav")) {
      std::cerr << "Failed to load assets/music/menu_sound.wav" << std::endl;
  }
  sf::Sound menuSound(menuSoundBuffer);

  // Load Background Music
  sf::Music bgMusic;
  if (!bgMusic.openFromFile("assets/music/cumbia.MP3")) {
      std::cerr << "Failed to load assets/music/cumbia.MP3" << std::endl;
  }
  bgMusic.setLooping(true);
  bgMusic.setVolume(50.0f); // Adjust volume if needed


  // Game State
  int lives = 3;
  int currentLevel = 1;
  enum State { MENU, PLAYING, DEATH_ANIM, LIVES_SCREEN, GAME_OVER, LEVEL_COMPLETE, GAME_WON };
  State currentState = MENU; // Start at MENU
  float stateTimer = 0.0f;

  // Session - usar LEVEL_WIDTH desde Level.hpp (3200px)
  std::unique_ptr<GameSession> session =
      std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT);

  // Camera
  sf::View camera(sf::FloatRect({0.f, 0.f}, {(float)WIDTH, (float)HEIGHT}));

  auto update = [&](float dt) {
    if (currentState == MENU) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            menuSound.play(); // Play menu sound
            bgMusic.play();   // Start background music
            currentState = PLAYING; 
            // Reset session just in case, or just start? 
            // Fresh start is better to ensure positions are correct.
            session = std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT);
        }
    } else if (currentState == PLAYING) {
      session->physics.step(dt);
      session->player->handleInput(dt);
      session->player->update(dt);
      session->level->update(dt);
      session->level->checkCollisions(*session->player);

      // Check if player wants to shoot fireball
      if (session->player->tryShootFireball()) {
        float dir = session->player->isFacingRight() ? 1.0f : -1.0f;
        sf::Vector2f pos = session->player->getPosition();
        // Spawn fireball slightly in front of Mario
        session->level->spawnFireball(pos.x + dir * 20.0f, pos.y - 10.0f, dir);
      }

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

      camera.setCenter({camX, camY});

      // Check Goal Reached
      if (session->level->isGoalReached()) {
        // Freeze player when goal is reached
        session->player->freeze();
        
        if (session->level->isGoalAnimComplete()) {
          // If level 2 is complete, go directly to GAME_WON
          if (currentLevel >= 2) {
            currentState = GAME_WON;
            bgMusic.stop(); // Stop music on win
            stateTimer = 5.0f; // Show "Juego Terminado" for 5 seconds
          } else {
            currentState = LEVEL_COMPLETE;
            stateTimer = 3.0f; // Show level screen for 3 seconds
          }
        }
      }

      // Check Death
      if (session->player->isDead()) {
        currentState = DEATH_ANIM;
      } else if (session->player->getPosition().y > (float)HEIGHT + 50.0f) {
        // Fell into void
        session->player->die();
        currentState = DEATH_ANIM;
      }

    } else if (currentState == DEATH_ANIM) {
      // Continue physics for destruction/falling
      session->physics.step(dt);
      session->player->update(dt);

      // Check Below Map
      if (session->player->getPosition().y > (float)HEIGHT + 100.0f) {
        lives--;
        if (lives > 0) {
          currentState = LIVES_SCREEN;
          stateTimer = 2.0f;
        } else {
          currentState = GAME_OVER;
          bgMusic.stop(); // Stop music immediately
          stateTimer = 3.0f;
        }
      }
    } else if (currentState == LIVES_SCREEN) {
      stateTimer -= dt;
      if (stateTimer <= 0.0f) {
        // Reset Level (keep same level number)
        session = std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT, currentLevel);
        currentState = PLAYING;
        camera.setCenter({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});
      }
    } else if (currentState == GAME_OVER) {
      stateTimer -= dt;
      if (stateTimer <= 0.0f) {
        bgMusic.stop(); // Stop music on game over
        // Restart Game to Menu
        currentState = MENU;
        lives = 3;
        currentLevel = 1; // Reset to level 1
        camera.setCenter({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});
      }
    } else if (currentState == LEVEL_COMPLETE) {
      stateTimer -= dt;
      if (stateTimer <= 0.0f) {
        // Check if game is complete (finished level 2)
        if (currentLevel >= 2) {
          currentState = GAME_WON;
          stateTimer = 5.0f; // Show "Juego Terminado" for 5 seconds
        } else {
          // Start next level
          currentLevel++;
          // Give 3 extra lives when reaching level 2
          lives += 3;
          session = std::make_unique<GameSession>((float)WIDTH, (float)HEIGHT, currentLevel);
          currentState = PLAYING;
          camera.setCenter({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});
        }
      }
    } else if (currentState == GAME_WON) {
      stateTimer -= dt;
      if (stateTimer <= 0.0f) {
        // Return to menu after winning
        currentState = MENU;
        lives = 3;
        currentLevel = 1;
        camera.setCenter({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});
      }
    }
  };

  auto render = [&]() {
    if (currentState == MENU) {
        window.window().setView(window.window().getDefaultView());
        window.window().draw(menuSprite);
    } else if (currentState == PLAYING || currentState == DEATH_ANIM) {
      window.window().setView(camera);
      session->level->draw(window.window());
      session->player->draw(window.window());

      // Draw HUD
      window.window().setView(window.window().getDefaultView());
      window.window().draw(helmetSprite); // Draw helmet icon
      std::string vidasText = (lives == 1) ? "Vida: " : "Vidas: ";
      hudText.setString(vidasText + std::to_string(lives));
      window.window().draw(hudText);
    } else {
      // Draw Black Screen with UI
      window.window().setView(window.window().getDefaultView());

      sf::RectangleShape blackScreen(sf::Vector2f((float)WIDTH, (float)HEIGHT));
      blackScreen.setFillColor(sf::Color::Black);
      
      // GAME_WON shows Final.png as background, GAME_OVER shows gameover.png
      if (currentState == GAME_WON) {
        window.window().draw(finalSprite);
      } else if (currentState == GAME_OVER) {
        window.window().draw(gameOverSprite);
      } else {
        window.window().draw(blackScreen);
      }

      // GAME_OVER already has full image, skip text for it
      if (currentState == GAME_OVER) {
        // Do nothing, image is already drawn
      } else {
        if (currentState == LIVES_SCREEN) {
          std::string vidasLabel = (lives == 1) ? " Vida" : " Vidas";
          uiText.setString(std::to_string(lives) + vidasLabel);
        } else if (currentState == LEVEL_COMPLETE) {
          uiText.setString("Nivel " + std::to_string(currentLevel + 1));
        } else if (currentState == GAME_WON) {
          uiText.setString("Juego Terminado");
        }

        // Center Text
        sf::FloatRect textBounds = uiText.getLocalBounds();
        uiText.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                          textBounds.position.y + textBounds.size.y / 2.0f});
        uiText.setPosition({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f});

        window.window().draw(uiText);
        
        // Draw "Gracias por Jugar" below main text on GAME_WON
        if (currentState == GAME_WON) {
          sf::Text thanksText(font);
          thanksText.setCharacterSize(30);
          thanksText.setFillColor(sf::Color::White);
          thanksText.setString("Gracias por Jugar");
          sf::FloatRect thanksBounds = thanksText.getLocalBounds();
          thanksText.setOrigin({thanksBounds.position.x + thanksBounds.size.x / 2.0f,
                                thanksBounds.position.y + thanksBounds.size.y / 2.0f});
          thanksText.setPosition({(float)WIDTH / 2.0f, (float)HEIGHT / 2.0f + 60.0f});
          window.window().draw(thanksText);
        }
        
        // Draw helmet icon next to lives text on LIVES_SCREEN
        if (currentState == LIVES_SCREEN) {
          sf::Sprite centeredHelmet(helmetTexture);
          centeredHelmet.setScale({0.8f, 0.8f}); // Larger for center screen
          // Position to the left of the text
          float helmetX = (float)WIDTH / 2.0f - textBounds.size.x / 2.0f - 60.0f;
          float helmetY = (float)HEIGHT / 2.0f - 25.0f;
          centeredHelmet.setPosition({helmetX, helmetY});
          window.window().draw(centeredHelmet);
        }
      }
    }
  };

  window.run(update, render);

  return 0;
}
