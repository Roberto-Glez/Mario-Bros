#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "Block.hpp"
#include "Enemy.hpp"
#include "FireFlower.hpp"
#include "Fireball.hpp"
#include "Goal.hpp"
#include "Goomba.hpp"
#include "Item.hpp"
#include "Koopa.hpp"
#include "Physics.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <memory>
#include <vector>


// Forward declaration
class Player;

class Level {
public:
  Level(Physics &physics, float width, float height, int levelNumber = 1);
  void draw(sf::RenderWindow &window);
  void update(float dt);
  void checkCollisions(Player &player);

  // Helper para la cámara
  float groundY() const;
  float getLevelWidth() const { return LEVEL_WIDTH; }

  // Fireballs
  void spawnFireball(float x, float y, float direction);

private:
  Physics &m_physics;

  sf::VertexArray m_groundVertices;
  sf::VertexArray m_groundVertices2; // Sección alternativa de suelo
  sf::VertexArray m_groundVertices3; // Tercera sección de suelo desde X=1216
  sf::VertexArray m_groundVertices4; // Cuarta sección de suelo desde X=4800
  sf::Texture m_texture;
  sf::Texture m_texture2; // Textura de plataformas.png

  std::vector<Block> m_blocks;
  std::vector<std::unique_ptr<Item>> m_items;
  std::vector<std::unique_ptr<Enemy>> m_enemies;
  std::vector<std::unique_ptr<Fireball>> m_fireballs;

  static constexpr int TILE_SIZE = 16;
  static constexpr float LEVEL_WIDTH = 6400.0f; // 8 pantallas de ancho

  b2BodyId m_groundBodyId;
  float m_width;
  float m_height;
  float m_groundY;
  int m_levelNumber;

  // Stomp cooldown to prevent ghost bounces and stomp loops
  float m_stompCooldown;
  static constexpr float STOMP_COOLDOWN_TIME = 0.5f; // 500ms cooldown

  // Plataformas sólidas (como el suelo)
  struct Platform {
    b2BodyId bodyId;
    sf::VertexArray vertices; // Para dibujar tiles con textura
    float x, y, width, height;
  };
  std::vector<Platform> m_platforms;

  // Bloques que matan al contacto
  struct KillBlock {
    b2BodyId bodyId;
    sf::RectangleShape shape;
    sf::Sprite sprite;
    float x, y, width, height;

    // Explicit constructor to initialize the Sprite with a texture reference
    // This bypasses the need for a default Sprite constructor if one is
    // missing.
    KillBlock(const sf::Texture &texture) : sprite(texture) {}
  };
  std::vector<KillBlock> m_killBlocks;

  // Decoraciones de fondo
  sf::Texture m_trapTexture;
  sf::Texture m_bgTexture;   // Nueva textura de fondo
  sf::Sprite m_bgSprite;     // Nuevo sprite de fondo
  sf::Sprite m_cornerSprite; // Sprite "spray" de la esquina
  std::vector<sf::Sprite> m_decorations;

  std::vector<sf::RectangleShape> m_coloredPlatforms;

  // Stomp sound effect
  sf::SoundBuffer m_stompSoundBuffer;
  sf::Sound m_stompSound;

  // Powerup sound effect
  sf::SoundBuffer m_powerupSoundBuffer;
  sf::Sound m_powerupSound;

  // Goal
  Goal m_goal;
  sf::SoundBuffer m_goalSoundBuffer;
  sf::Sound m_goalSound;

public:
  bool isGoalReached() const { return m_goal.isTriggered(); }
  bool isGoalAnimComplete() const { return m_goal.isAnimationComplete(); }
};

#endif // LEVEL_HPP