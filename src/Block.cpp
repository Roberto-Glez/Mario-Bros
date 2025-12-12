#include "Block.hpp"
#include <iostream>

Block::Block(Physics &physics, float x, float y)
    : m_physics(physics), m_type(Type::Question), m_active(true),
      m_animTimer(0.0f), m_frame(0), m_sprite(m_texture) {
  if (!m_texture.loadFromFile("assets/images/blocks.png")) {
    std::cerr << "Error loading blocks.png" << std::endl;
  }
  m_sprite.setTexture(m_texture);

  // Sprite del bloque - posición (0, 112) tamaño 16x16
  m_sprite.setTextureRect(sf::IntRect({0, 112}, {16, 16}));
  m_sprite.setOrigin({8, 8});      // Center
  m_sprite.setScale({2.0f, 2.0f}); // Visual scale
  m_sprite.setPosition({x, y});

  // Physics Body (Static)
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_staticBody;
  bodyDef.position = (b2Vec2){x / Physics::SCALE, y / Physics::SCALE};

  m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

  b2Polygon box = b2MakeBox((16.0f / 2.0f) / Physics::SCALE,
                            (16.0f / 2.0f) / Physics::SCALE); // 16x16
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  // shapeDef.friction = 1.0f;

  b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
}

void Block::update(float dt) {
  if (m_type == Type::Question) {
    m_animTimer += dt;
    if (m_animTimer > 0.2f) {
      m_animTimer = 0.0f;
      m_frame = (m_frame + 1) % 3; // 3 frames of animation
      // Animación del bloque - 3 frames empezando en (0, 112)
      m_sprite.setTextureRect(sf::IntRect({m_frame * 16, 112}, {16, 16}));
    }
  }
}

bool Block::hit() {
  if (m_type == Type::Question) {
    m_type = Type::Empty;
    // Change to Empty Block (Brown Empty is usually at 48, 0)
    // Or if there is a green empty block at 48, 32? Let's try 48, 32.
    m_sprite.setTextureRect(sf::IntRect({48, 32}, {16, 16}));
    // Or just use the brown one if green empty doesn't exist
    // m_sprite.setTextureRect(sf::IntRect(48, 0, 16, 16));
    return true;  // First hit - spawn item
  }
  return false;  // Already hit - no item
}

void Block::draw(sf::RenderWindow &window) { window.draw(m_sprite); }

sf::FloatRect Block::getBounds() const { return m_sprite.getGlobalBounds(); }

sf::Vector2f Block::getPosition() const { return m_sprite.getPosition(); }
