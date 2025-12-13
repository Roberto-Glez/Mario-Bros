#include "Block.hpp"
#include <iostream>

Block::Block(Physics &physics, float x, float y)
    : m_physics(&physics),  m_sprite(m_texture), m_type(Type::Question), m_active(true),
      m_animTimer(0.0f), m_frame(0) {
  // Cargar textura primero
  if (!m_texture.loadFromFile("assets/images/bloque_poder.png")) {
    std::cerr << "Error loading bloque_poder.png" << std::endl;
  }

  // Recrear sprite con la textura cargada
  m_sprite = sf::Sprite(m_texture);

  // Sprite del bloque - Estado inicial (Question)
  // Usuario especificó: empieza en (2,3) y es de 55x42
  m_sprite.setTextureRect(sf::IntRect({2, 3}, {55, 42}));
  m_sprite.setOrigin({27.5f, 21.0f}); // Center (55/2, 42/2)
  
  // Escalar para que sea exactamente de 32x32 (tamaño de la cuadrícula)
  m_sprite.setScale({32.0f / 55.0f, 32.0f / 42.0f}); 
  
  m_sprite.setPosition({x, y});

  // Physics Body (Static)
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_staticBody;
  bodyDef.position = (b2Vec2){x / Physics::SCALE, y / Physics::SCALE};

  m_bodyId = b2CreateBody(m_physics->worldId(), &bodyDef);

  // Box size 32x32 (Half-width 16)
  b2Polygon box = b2MakeBox((32.0f / 2.0f) / Physics::SCALE,
                            (32.0f / 2.0f) / Physics::SCALE); 
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  // shapeDef.friction = 1.0f;

  b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
}

// Move Constructor
Block::Block(Block&& other) noexcept
    : m_physics(other.m_physics),
      m_bodyId(other.m_bodyId),
      m_texture(std::move(other.m_texture)), 
      m_sprite(std::move(other.m_sprite)),
      m_type(other.m_type),
      m_active(other.m_active),
      m_animTimer(other.m_animTimer),
      m_frame(other.m_frame) {
  
  // Re-link sprite to the new texture location in memory
  m_sprite.setTexture(m_texture);
  
  other.m_bodyId = b2_nullBodyId; 
  other.m_physics = nullptr;
}

// Move Assignment
Block& Block::operator=(Block&& other) noexcept {
  if (this != &other) {
    m_physics = other.m_physics;
    m_bodyId = other.m_bodyId;
    m_texture = std::move(other.m_texture);
    m_sprite = std::move(other.m_sprite);
    m_type = other.m_type;
    m_active = other.m_active;
    m_animTimer = other.m_animTimer;
    m_frame = other.m_frame;

    // Re-link sprite
    m_sprite.setTexture(m_texture);

    other.m_bodyId = b2_nullBodyId;
    other.m_physics = nullptr;
  }
  return *this;
}

void Block::update(float dt) {
  // Animación removida temporalmente ya que el usuario especificó solo dos estados estáticos
  if (m_type == Type::Question) {
     m_sprite.setTextureRect(sf::IntRect({2, 3}, {55, 42}));
  }
}

bool Block::hit() {
  if (m_type == Type::Question) {
    m_type = Type::Empty;
    // Cambiar a bloque vacío (segundo sprite)
    // Usuario pidió mover el recorte a la derecha. 
    // Anterior: 59. Ajustando a 84 (aprox +25px).
    m_sprite.setTextureRect(sf::IntRect({84, 3}, {55, 42}));
    return true; // First hit - spawn item
  }
  return false; // Already hit - no item
}

void Block::draw(sf::RenderWindow &window) { window.draw(m_sprite); }

sf::FloatRect Block::getBounds() const { return m_sprite.getGlobalBounds(); }

sf::Vector2f Block::getPosition() const { return m_sprite.getPosition(); }
