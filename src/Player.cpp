#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath> // Para std::abs
#include <iostream>

Player::Player(Physics &physics, float startX, float startY)
    : m_physics(physics), m_sprite(m_texture), m_width(32.0f), m_height(32.0f),
      m_canJump(false), m_isBig(false), m_isFireMario(false), m_isDead(false),
      m_isInvulnerable(false), m_invulnerableTimer(0.0f),
      m_animationTimer(0.0f), m_groundTimer(0.0f), m_runTimer(0.0f),
      m_currentFrame(0), m_facingRight(true), m_state(State::Idle),
      m_fireballCooldown(0.0f), m_throwTimer(0.0f), m_isThrowing(false) {
  // ... (Constructor content unchanged) ...
  // Load Texture
  if (!m_texture.loadFromFile("assets/images/mario_chiquito.png")) {
    std::cerr << "Error loading mario_chiquito.png" << std::endl;
    // Handle error? For now just continue, sprite will be white
  }
  if (!m_bigTexture.loadFromFile("assets/images/mario_grande.png")) {
    std::cerr << "Error loading mario_grande.png" << std::endl;
  }
  if (!m_fireTexture.loadFromFile("assets/images/mario_fuego.png")) {
    std::cerr << "Error loading mario_fuego.png" << std::endl;
  }
  m_sprite.setTexture(m_texture);

  // Set initial frame (Idle = 0)
  // Height 19, Origin Y 10 as per calibration V2
  m_sprite.setTextureRect(sf::IntRect({0, 0}, {16, 19}));

  // Center origin X (8), Align Y (10)
  m_sprite.setOrigin({16.0f / 2.0f, 10.0f});

  // Scale visual
  m_sprite.setScale({2.0f, 2.0f});

  // Definición del cuerpo (v3)
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = (b2Vec2){startX / Physics::SCALE, startY / Physics::SCALE};
  bodyDef.fixedRotation = true;

  // Crear el cuerpo usando el ID del mundo
  m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

  // Definir la forma (Caja)
  // Use slightly adjusting sizing for physics if we want it to match sprite
  // better But keeping original 32x48 for now to avoid breaking existing level
  // collisions
  b2Polygon dynamicBox = b2MakeBox((m_width / 2.0f) / Physics::SCALE,
                                   (m_height / 2.0f) / Physics::SCALE);

  // Definir la "fixture" (propiedades físicas)
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  // shapeDef.friction = 0.3f;

  // Unir forma al cuerpo
  b2CreatePolygonShape(m_bodyId, &shapeDef, &dynamicBox);
}

void Player::handleInput(float dt) {
  if (m_isDead)
    return;

  b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
  float desiredVel = 0.0f;

  // Acceleration Constants
  const float WALK_SPEED = 5.0f;  // Slightly faster walk
  const float RUN_SPEED = 10.0f;  // Faster run
  const float ACCEL_DELAY = 0.3f; // Faster momentum buildup
  const float ACCEL_RAMP = 0.7f;  // Quicker acceleration to max speed

  // Input Handling
  bool isMoving = false;
  bool leftInput = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
  bool rightInput = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
  bool downInput = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
  bool isSkidding = false;
  bool isCrouching = downInput && m_isBig &&
                     m_canJump; // Crouch only if Big Mario and on ground

  if (leftInput) {
    if (vel.x > 0.5f)
      isSkidding = true;
    else {
      isMoving = true;
      m_facingRight = false;
    }
  }
  if (rightInput) {
    if (vel.x < -0.5f)
      isSkidding = true;
    else {
      isMoving = true;
      m_facingRight = true;
    }
  }

  // Timer Logic
  if (isMoving && !isSkidding) {
    m_runTimer += dt;
  } else {
    m_runTimer = 0.0f; // Reset momentum on stop or skid
  }

  // Calculate Speed based on Timer
  float currentSpeed = WALK_SPEED;
  if (m_runTimer > ACCEL_DELAY) {
    // Ramp from WALK to RUN
    // fraction goes from 0.0 to 1.0 over ACCEL_RAMP seconds
    float fraction = (m_runTimer - ACCEL_DELAY) / ACCEL_RAMP;
    if (fraction > 1.0f)
      fraction = 1.0f;

    currentSpeed = WALK_SPEED + (RUN_SPEED - WALK_SPEED) * fraction;
  }

  // Apply Velocity
  if (isCrouching) {
    desiredVel = vel.x * 0.92f; // Gradual deceleration while crouching
  } else if (isSkidding) {
    desiredVel = vel.x * 0.90f; // Braking friction
  } else {
    if (leftInput)
      desiredVel = -currentSpeed;
    else if (rightInput)
      desiredVel = currentSpeed;
    else
      desiredVel = vel.x * 0.5f; // Stop friction
  }

  // Determine State logic for animation
  // Check Jump
  if (!m_canJump) {
    m_state = State::Jumping;
  } else if (isCrouching) {
    m_state = State::Crouching;
  } else if (isSkidding) {
    m_state = State::Braking;
  } else if (std::abs(vel.x) > 0.1f) {
    m_state = State::Running;
  } else {
    m_state = State::Idle;
  }

  // Physics Movement
  float velChange = desiredVel - vel.x;
  float impulse = b2Body_GetMass(m_bodyId) * velChange;
  b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){impulse, 0.0f}, true);

  // Salto - SOLO con Up (Space ahora es para fuego)
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && m_canJump) {
    float jumpImpulse = -b2Body_GetMass(m_bodyId) * 11.0f;
    b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){0.0f, jumpImpulse},
                                      true);
    m_canJump = false;
    m_state = State::Jumping;
  }
}

void Player::update(float dt) {
  // Invulnerability Timer
  if (m_isInvulnerable) {
    m_invulnerableTimer += dt;
    if (m_invulnerableTimer >= 2.0f) {
      m_isInvulnerable = false;
      m_invulnerableTimer = 0.0f;
      m_sprite.setColor(sf::Color::White);
    } else {
      // Blink effect
      int blink = (int)(m_invulnerableTimer * 20.0f) % 2;
      m_sprite.setColor(blink ? sf::Color(255, 255, 255, 100)
                              : sf::Color::White);
    }
  }

  b2Vec2 pos = b2Body_GetPosition(m_bodyId);
  b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

  // Actualizar gráfico SFML
  m_sprite.setPosition({pos.x * Physics::SCALE, pos.y * Physics::SCALE});

  // Scale flipping based on direction
  if (m_facingRight) {
    m_sprite.setScale({2.0f, 2.0f});
  } else {
    m_sprite.setScale({-2.0f, 2.0f});
  }

  // Chequeo de suelo mejorado (Hysteresis/Timer)
  // El problema: En el pico del salto, vel.y es ~0, lo que activaba m_canJump y
  // cambiaba el estado a Running. Solución: Requerir que la velocidad sea baja
  // durante un breve tiempo (ej. 3 frames = 0.05s).
  if (std::abs(vel.y) < 0.1f) {
    m_groundTimer += dt;
  } else {
    m_groundTimer = 0.0f;
  }

  // Solo activamos salto si el timer supera el umbral
  if (m_groundTimer > 0.05f) {
    m_canJump = true;
  } else {
    m_canJump = false;
  }

  // Variable gravity for snappier jumps (Mario-style)
  // Apply extra downward force when falling or when jump button released
  bool jumpHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);

  // Fireball cooldown
  if (m_fireballCooldown > 0.0f) {
    m_fireballCooldown -= dt;
  }

  // Throw animation timer
  if (m_isThrowing) {
    m_throwTimer += dt;
    if (m_throwTimer >= THROW_ANIM_DURATION) {
      m_isThrowing = false;
      m_throwTimer = 0.0f;
    }
  }

  if (vel.y > 0.5f) {
    // Falling - apply extra gravity for faster descent
    float fallMultiplier = 1.5f;
    b2Body_ApplyForceToCenter(
        m_bodyId,
        (b2Vec2){0.0f, b2Body_GetMass(m_bodyId) * 15.0f * fallMultiplier},
        true);
  } else if (vel.y < -0.5f && !jumpHeld) {
    // Rising but jump released - cut jump short
    float lowJumpMultiplier = 2.0f;
    b2Body_ApplyForceToCenter(
        m_bodyId,
        (b2Vec2){0.0f, b2Body_GetMass(m_bodyId) * 15.0f * lowJumpMultiplier},
        true);
  }

  updateAnimation(dt);
}

void Player::updateAnimation(float dt) {
  // Dynamic Animation Speed
  // Default: 0.1s (10 fps)
  // Max Run: 0.05s (20 fps) "He fits 2 steps in the time of 1"

  float frameTime = 0.1f;
  if (m_state == State::Running) {
    // Accelerate animation as we run faster
    const float MAX_SPEED_TIME = 2.5f; // Timer value for max speed
    float factor = std::min(m_runTimer / MAX_SPEED_TIME, 1.0f);

    // Lerp from 0.1 to 0.05
    frameTime = 0.1f - (0.05f * factor);
  }

  int startFrame = 0;
  int numFrames = 1;

  // Configure frames based on state
  // Indices:
  // 0: Idle
  // 1: Brake
  // 2: Jump
  // 4,5,6: Run (User said 5,6,7. Assuming 1-based index in user request =>
  // 4,5,6)

  switch (m_state) {
  case State::Idle:
    startFrame = 0;
    numFrames = 1;
    break;
  case State::Braking:
    startFrame = 1;
    numFrames = 1;
    break;
  case State::Jumping:
    startFrame = 2; // User said 3rd sprite
    numFrames = 1;
    break;
  case State::Running:
    startFrame = 4; // User said 5,6,7 => Indices 4,5,6
    numFrames = 3;
    break;
  case State::Crouching:
    startFrame = 3; // Sprite index 3 (4th sprite) for crouch
    numFrames = 1;
    break;
  case State::Throwing:
    // Handled separately for Fire Mario
    startFrame = 0;
    numFrames = 1;
    break;
  case State::Dead:
    // Handled separately below
    break;
  }

  m_animationTimer += dt;
  if (m_animationTimer >= frameTime) {
    m_animationTimer = 0.0f;
    m_currentFrame++;
    if (m_currentFrame >= numFrames) {
      m_currentFrame = 0;
    }
  }

  int textureIndex = startFrame + m_currentFrame;

  // Configuración ajustada visualmente según reporte
  int tileWidth = 16;
  int tileHeight =
      20;          // Más alto para incluir piernas (pero reducido a 19 abajo)
  int gap = 1;     // Espacio entre sprites
  int offsetX = 0; // Start offset

  int finalStride = 18;
  int left = textureIndex * finalStride;

  if (m_isBig) {
    // Big Mario Animation (also used for Fire Mario - same layout)
    // Height adjusted to 33 to prevent foot clipping
    // Origin adjusted to (8, 17.5) to align vertically with physics center
    // - Physics Box Height ~30. Center to Bottom = 15.
    // - Sprite Height 33. Feet at 33.
    // - We want Feet (33) to align with Body Bottom (Center + 15).
    // - So Center should align with Sprite 18 (33 - 15).
    // - Let's use Origin 17.5 to center it well.

    // Check if Fire Mario is throwing
    if (m_isFireMario && m_isThrowing) {
      // Special throwing animation for Fire Mario
      b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
      bool isMoving = std::abs(vel.x) > 0.5f;

      if (isMoving) {
        // Running while throwing: sprites at (288, 2) and onwards
        int throwRunFrame = m_currentFrame % 3;
        int throwLeft = 288 + throwRunFrame * 18;
        m_sprite.setTextureRect(sf::IntRect({throwLeft, 2}, {18, 35}));
      } else {
        // Idle throwing: sprite at (270, 2) - shifted 1px right to crop left
        // edge
        m_sprite.setTextureRect(sf::IntRect({270, 2}, {18, 35}));
      }
      m_sprite.setOrigin(
          {18.0f / 2.0f, 20.0f}); // Lower origin for throwing sprites
    } else {
      // Normal animation
      m_sprite.setTextureRect(sf::IntRect({left, 0}, {18, 35}));
      m_sprite.setOrigin({18.0f / 2.0f, 22.0f});
    }

    // Switch texture based on fire state
    if (m_isFireMario) {
      m_sprite.setTexture(m_fireTexture);
    } else {
      m_sprite.setTexture(m_bigTexture);
    }
  } else {
    // Small Mario Animation
    // Ajuste final V2:
    // - Height 20 tenía un pixel negro abajo. -> Bajamos a 19.
    // - Levitación:
    //   Si Feet están en y=18 (último pixel válido en altura 19), y queremos
    //   distancia visual 16px. (18 - OriginY) * 2.0 = 16. 18 - OriginY = 8.
    //   OriginY = 10.

    m_sprite.setTextureRect(sf::IntRect({left, 0}, {16, 19}));
    m_sprite.setOrigin({16.0f / 2.0f, 10.0f});
  }

  // Special override only for DEAD state
  if (m_state == State::Dead) {
    // User requested: Sprite 4 starts at pixel (55, 2).
    // Assuming 16x16 size for the dead sprite.
    m_sprite.setTextureRect(sf::IntRect({55, 2}, {16, 16}));
    // Reset origin to match correct center for dying animation
    m_sprite.setOrigin({16.0f / 2.0f, 10.0f});
  }
}

void Player::draw(sf::RenderWindow &window) { window.draw(m_sprite); }

sf::Vector2f Player::getPosition() const { return m_sprite.getPosition(); }

sf::FloatRect Player::getBounds() const { return m_sprite.getGlobalBounds(); }

b2Vec2 Player::getVelocity() const {
  if (b2Body_IsValid(m_bodyId)) {
    return b2Body_GetLinearVelocity(m_bodyId);
  }
  return {0.0f, 0.0f};
}

void Player::grow() {
  if (m_isBig)
    return; // Already big

  m_isBig = true;
  m_sprite.setTexture(m_bigTexture);

  // Resize Physics Body
  // Destroy old
  b2Vec2 pos = b2Body_GetPosition(m_bodyId);
  b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
  b2DestroyBody(m_bodyId);

  // Create new (Taller)
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  // Adjust Y position upward to account for taller hitbox
  // Big Mario hitbox: 28x52 pixels (scaled from sprite ~14x26 base * 2)
  bodyDef.position = (b2Vec2){pos.x, pos.y - (26.0f / Physics::SCALE)};
  bodyDef.fixedRotation = true;

  m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

  // Make Box for Big Mario: 28x52 pixels to better match sprite collision
  b2Polygon box = b2MakeBox((28.0f / 2.0f) / Physics::SCALE,
                            (52.0f / 2.0f) / Physics::SCALE);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  // shapeDef.friction = 0.3f;

  b2CreatePolygonShape(m_bodyId, &shapeDef, &box);

  b2Body_SetLinearVelocity(m_bodyId, vel); // Maintain momentum
}

void Player::becomeFireMario() {
  if (!m_isBig) {
    // Small Mario grabs Fire Flower: grow first, then become Fire
    grow();
    // Don't return - continue to become Fire Mario
  }

  if (m_isFireMario)
    return; // Already fire mario

  m_isFireMario = true;
  m_sprite.setTexture(m_fireTexture);
}

void Player::bounce() {
  // Small bounce after stomping enemy - set velocity directly
  b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
  // Set upward velocity directly (negative Y = up)
  b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){vel.x, -6.0f});
  m_state = State::Jumping;
}

void Player::takeDamage() {
  if (m_isInvulnerable || m_isDead)
    return;

  if (m_isFireMario) {
    // Fire Mario -> Big Mario
    m_isFireMario = false;
    m_isInvulnerable = true;
    m_invulnerableTimer = 0.0f;

    // Switch back to Big texture
    m_sprite.setTexture(m_bigTexture);
    // Still big, no physics change needed
  } else if (m_isBig) {
    m_isBig = false;
    m_isInvulnerable = true;
    m_invulnerableTimer = 0.0f;

    // Revert to Small Texture
    m_sprite.setTexture(m_texture);

    // Revert Physics Body to Small
    b2Vec2 pos = b2Body_GetPosition(m_bodyId);
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

    b2DestroyBody(m_bodyId);

    // Create Small Body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = pos;
    bodyDef.fixedRotation = true;

    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

    // Original Small Box
    b2Polygon dynamicBox = b2MakeBox((m_width / 2.0f) / Physics::SCALE,
                                     (m_height / 2.0f) / Physics::SCALE);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    // shapeDef.friction = 0.3f;

    b2CreatePolygonShape(m_bodyId, &shapeDef, &dynamicBox);

    b2Body_SetLinearVelocity(m_bodyId, vel);

    // Sound effect here if we had audio
  } else {
    die();
  }
}

void Player::die() {
  if (m_isDead)
    return;

  m_isDead = true;
  m_state = State::Dead;

  // Jump up
  // Jump up
  b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
  b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){0.0f, -20.0f}); // Jump (Higher)

  // Disable collisions by destroying shapes but keeping body for logic?
  // Or just set shape filter.
  // Box2D v3: we can just destroy shapes.
  // But b2Body needs shape to have mass?
  // If we destroy shape, mass becomes 0? b2Body w/o shape is just a point.
  // If it's dynamic, it might not move without mass if forces dependent on mass
  // are applied? Actually gravity is applied as force = m * g. We should better
  // use filter to collide with nothing.

  // Changing filter requires accessing the shape ID.
  // Since I don't store shape ID, I will just destroy body and create a new one
  // that collides with nothing.

  b2Vec2 pos = b2Body_GetPosition(m_bodyId);
  b2DestroyBody(m_bodyId);

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = pos;
  bodyDef.fixedRotation = true;
  // Set linear velocity
  b2BodyId newBody = b2CreateBody(m_physics.worldId(), &bodyDef);
  b2Body_SetLinearVelocity(newBody, (b2Vec2){0.0f, -20.0f}); // Jump (Higher)

  // Add shape with interaction filter that hits nothing
  b2Polygon box = b2MakeBox((m_width / 2.0f) / Physics::SCALE,
                            (m_height / 2.0f) / Physics::SCALE);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  shapeDef.filter.categoryBits = 0; // Collide with nothing
  shapeDef.filter.maskBits = 0;

  b2CreatePolygonShape(newBody, &shapeDef, &box);

  m_bodyId = newBody;
}

bool Player::tryShootFireball() {
  // Only Fire Mario can shoot
  if (!m_isFireMario || m_isDead)
    return false;

  // Check cooldown
  if (m_fireballCooldown > 0.0f)
    return false;

  // Check if Space is pressed
  if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
    return false;

  // Fire!
  m_fireballCooldown = FIREBALL_COOLDOWN;
  m_isThrowing = true;
  m_throwTimer = 0.0f;

  return true;
}