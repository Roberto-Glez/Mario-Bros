#include "Goal.hpp"
#include <iostream>

Goal::Goal()
    : m_sprite(m_texture), m_poleSprite(m_texture), m_x(0), m_y(0), m_triggered(false), 
      m_animComplete(false), m_animTimer(0.0f), m_frame(0), m_totalAnimTime(0.0f) {
}

void Goal::init(float x, float y) {
  m_x = x;
  m_y = y;
  
  // Load texture
  if (!m_texture.loadFromFile("assets/images/Goal.png")) {
    std::cerr << "Error loading Goal.png" << std::endl;
  }
  
  // Set up flag sprite (animated) - starts static on frame 1
  m_sprite.setTexture(m_texture);
  m_sprite.setTextureRect(sf::IntRect({SPRITE_X, SPRITE_Y}, {SPRITE_WIDTH, SPRITE_HEIGHT}));
  m_sprite.setOrigin({SPRITE_WIDTH / 2.0f, SPRITE_HEIGHT});
  m_sprite.setScale({2.0f, 2.0f});
  m_sprite.setPosition({x, y});
  
  // Set up pole sprite (static, always visible behind flag)
  m_poleSprite.setTexture(m_texture);
  m_poleSprite.setTextureRect(sf::IntRect({POLE_X, POLE_Y}, {POLE_WIDTH, POLE_HEIGHT}));
  m_poleSprite.setOrigin({POLE_WIDTH / 2.0f, POLE_HEIGHT});
  m_poleSprite.setScale({2.0f, 2.0f});
  m_poleSprite.setPosition({x, y});
}

void Goal::update(float dt) {
  if (!m_triggered || m_animComplete) {
    return;
  }
  
  m_totalAnimTime += dt;
  m_animTimer += dt;
  
  if (m_animTimer >= FRAME_TIME) {
    m_animTimer = 0.0f;
    m_frame = (m_frame + 1) % NUM_FRAMES; // Cycles through 0, 1, 2, 3
    
    // Update texture rect for animation (sprite 1 to 4)
    if (m_frame == 3) {
      // Frame 4 - cut 7 pixels from left (114 + 7 = 121)
      m_sprite.setTextureRect(sf::IntRect({121, 6}, {25, 49}));
      m_sprite.setOrigin({25 / 2.0f, 49.0f});
    } else {
      int frameX = SPRITE_X + m_frame * (SPRITE_WIDTH + SPRITE_GAP);
      m_sprite.setTextureRect(sf::IntRect({frameX, SPRITE_Y}, {SPRITE_WIDTH, SPRITE_HEIGHT}));
      m_sprite.setOrigin({SPRITE_WIDTH / 2.0f, SPRITE_HEIGHT});
    }
  }
  
  // Check if animation is complete
  if (m_totalAnimTime >= ANIM_DURATION) {
    m_animComplete = true;
  }
}

void Goal::draw(sf::RenderWindow &window) {
  // Draw pole first (behind flag)
  window.draw(m_poleSprite);
  // Draw flag/animation on top
  window.draw(m_sprite);
}

void Goal::trigger() {
  if (!m_triggered) {
    m_triggered = true;
    m_animTimer = 0.0f;
    m_frame = 0;
    m_totalAnimTime = 0.0f;
  }
}

sf::FloatRect Goal::getBounds() const {
  return m_sprite.getGlobalBounds();
}
