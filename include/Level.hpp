#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "Block.hpp"
#include "Enemy.hpp"
#include "Goomba.hpp"
#include "Item.hpp"
#include "Koopa.hpp"
#include "Physics.hpp"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <memory>
#include <vector>

// Forward declaration
class Player;

class Level {
public:
  Level(Physics &physics, float width, float height);
  void draw(sf::RenderWindow &window);
  void update(float dt);
  void checkCollisions(Player &player);

  // Helper para la cámara
  float groundY() const;
  float getLevelWidth() const { return LEVEL_WIDTH; }

private:
  Physics &m_physics;

  sf::VertexArray m_groundVertices;
  sf::Texture m_texture;

  std::vector<Block> m_blocks;
  std::vector<std::unique_ptr<Item>> m_items;
  std::vector<std::unique_ptr<Enemy>> m_enemies;

  static constexpr int TILE_SIZE = 16;
  static constexpr float LEVEL_WIDTH = 3200.0f; // 4 pantallas de ancho

  b2BodyId m_groundBodyId;
  float m_width;
  float m_height;
  float m_groundY;

  // Stomp cooldown to prevent ghost bounces and stomp loops
  float m_stompCooldown;
  static constexpr float STOMP_COOLDOWN_TIME = 0.5f; // 500ms cooldown
};

#endif // LEVEL_HPP