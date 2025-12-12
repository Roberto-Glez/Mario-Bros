#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Physics.hpp"
#include <SFML/Graphics.hpp>

class Player {
public:
  Player(Physics &physics, float startX, float startY);
  void handleInput(float dt); // Added dt for acceleration timer
  void update(float dt);
  void draw(sf::RenderWindow &window);
  void grow();
  void becomeFireMario();
  void bounce(); // Bounce after stomping enemy
  bool isBig() const { return m_isBig; }
  bool isFireMario() const { return m_isFireMario; }
  // Método helper para la cámara
  sf::Vector2f getPosition() const;
  sf::FloatRect getBounds() const;
  b2Vec2 getVelocity() const;

  // Damage & Life
  void takeDamage();
  bool isDead() const { return m_isDead; }
  void die();

  // Fireball
  bool tryShootFireball(); // Returns true if fireball should be spawned
  bool isFacingRight() const { return m_facingRight; }

private:
  void updateAnimation(float dt);

  Physics &m_physics;
  b2BodyId m_bodyId;

  sf::Texture m_texture;
  sf::Texture m_bigTexture;
  sf::Texture m_fireTexture;
  sf::Sprite m_sprite;

  float m_width;
  float m_height;
  bool m_canJump;
  bool m_isBig;
  bool m_isFireMario;

  // Death & Invulnerability
  bool m_isDead;
  bool m_isInvulnerable;
  float m_invulnerableTimer;

  // Animation state
  float m_animationTimer;
  float m_groundTimer; // To filter jump apex
  float m_runTimer;    // Momentum timer
  int m_currentFrame;
  bool m_facingRight;
  enum class State {
    Idle,
    Running,
    Jumping,
    Braking,
    Crouching,
    Throwing,
    Dead
  } m_state;

  // Fireball shooting
  float m_fireballCooldown;
  float m_throwTimer; // Timer for throw animation
  bool m_isThrowing;
  static constexpr float FIREBALL_COOLDOWN = 0.5f;
  static constexpr float THROW_ANIM_DURATION = 0.15f;
};

#endif // PLAYER_HPP
