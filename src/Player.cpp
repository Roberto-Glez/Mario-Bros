#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath> // Para std::abs
#include <iostream>

Player::Player(Physics& physics, float startX, float startY)
: m_physics(physics), m_width(32.0f), m_height(32.0f), m_canJump(false),
  m_animationTimer(0.0f), m_groundTimer(0.0f), m_runTimer(0.0f), m_currentFrame(0), m_facingRight(true), m_state(State::Idle)
{
    // ... (Constructor content unchanged) ...
    // Load Texture
    if (!m_texture.loadFromFile("assets/images/mario_chiquito.png")) {
        std::cerr << "Error loading mario_chiquito.png" << std::endl;
        // Handle error? For now just continue, sprite will be white
    }
    m_sprite.setTexture(m_texture);
    
    // Set initial frame (Idle = 0)
    // Height 19, Origin Y 10 as per calibration V2
    m_sprite.setTextureRect(sf::IntRect(0, 0, 16, 19));
    
    // Center origin X (8), Align Y (10)
    m_sprite.setOrigin(16.0f / 2.0f, 10.0f);
    
    // Scale visual
    m_sprite.setScale(2.0f, 2.0f);


    // Definición del cuerpo (v3)
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){startX / Physics::SCALE, startY / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    // Crear el cuerpo usando el ID del mundo
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

    // Definir la forma (Caja)
    // Use slightly adjusting sizing for physics if we want it to match sprite better
    // But keeping original 32x48 for now to avoid breaking existing level collisions
    b2Polygon dynamicBox = b2MakeBox((m_width / 2.0f) / Physics::SCALE, (m_height / 2.0f) / Physics::SCALE);

    // Definir la "fixture" (propiedades físicas)
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;

    // Unir forma al cuerpo
    b2CreatePolygonShape(m_bodyId, &shapeDef, &dynamicBox);
}

void Player::handleInput(float dt)
{
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
    float desiredVel = 0.0f;
    
    // Acceleration Constants
    const float WALK_SPEED = 4.0f;
    const float RUN_SPEED = 8.0f;     // "Faster"
    const float ACCEL_DELAY = 1.0f;   // 0 to 1.0s: Walk (5 steps approx)
    const float ACCEL_RAMP = 1.5f;    // 1.0s to 2.5s: Acceleration phase (Total ~10 steps)

    // Input Handling
    bool isMoving = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        isMoving = true;
        m_facingRight = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        isMoving = true;
        m_facingRight = true;
    }

    // Timer Logic
    if (isMoving) {
        m_runTimer += dt;
    } else {
        m_runTimer = 0.0f; // Reset momentum on stop
    }

    // Calculate Speed based on Timer
    float currentSpeed = WALK_SPEED;
    if (m_runTimer > ACCEL_DELAY) {
        // Ramp from WALK to RUN
        // fraction goes from 0.0 to 1.0 over ACCEL_RAMP seconds
        float fraction = (m_runTimer - ACCEL_DELAY) / ACCEL_RAMP;
        if (fraction > 1.0f) fraction = 1.0f;
        
        currentSpeed = WALK_SPEED + (RUN_SPEED - WALK_SPEED) * fraction;
    }

    // Apply Velocity
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        desiredVel = -currentSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        desiredVel = currentSpeed;
    }

    // Determine State logic for animation
    // Check Jump
    if (!m_canJump) {
        m_state = State::Jumping;
    } else {
        // On ground
        if (std::abs(vel.x) > 0.1f) {
            // Check for braking (moving opposite to velocity)
            if ((desiredVel > 0 && vel.x < -0.5f) || (desiredVel < 0 && vel.x > 0.5f)) {
                 m_state = State::Braking;
            } else {
                 m_state = State::Running;
            }
        } else {
            m_state = State::Idle;
        }
    }

    // Physics Movement
    float velChange = desiredVel - vel.x;
    float impulse = b2Body_GetMass(m_bodyId) * velChange;
    b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){impulse, 0.0f}, true);

    // Salto
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) && m_canJump) {
        float jumpImpulse = -b2Body_GetMass(m_bodyId) * 6.0f;
        b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){0.0f, jumpImpulse}, true);
        m_canJump = false;
        m_state = State::Jumping;
    }
}

void Player::update(float dt)
{
    b2Vec2 pos = b2Body_GetPosition(m_bodyId);
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

    // Actualizar gráfico SFML
    m_sprite.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
    
    // Scale flipping based on direction
    if (m_facingRight) {
        m_sprite.setScale(2.0f, 2.0f);
    } else {
        m_sprite.setScale(-2.0f, 2.0f);
    }

    // Chequeo de suelo mejorado (Hysteresis/Timer)
    // El problema: En el pico del salto, vel.y es ~0, lo que activaba m_canJump y cambiaba el estado a Running.
    // Solución: Requerir que la velocidad sea baja durante un breve tiempo (ej. 3 frames = 0.05s).
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
    // 4,5,6: Run (User said 5,6,7. Assuming 1-based index in user request => 4,5,6)
    
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
    int tileHeight = 20; // Más alto para incluir piernas (pero reducido a 19 abajo)
    int gap = 1;         // Espacio entre sprites
    int offsetX = 0;     // Start offset

    
    int finalStride = 18;
    int left = textureIndex * finalStride;

    // Ajuste final V2:
    // - Height 20 tenía un pixel negro abajo. -> Bajamos a 19.
    // - Levitación:
    //   Si Feet están en y=18 (último pixel válido en altura 19), y queremos distancia visual 16px.
    //   (18 - OriginY) * 2.0 = 16.
    //   18 - OriginY = 8.
    //   OriginY = 10.
    
    m_sprite.setTextureRect(sf::IntRect(left, 0, 16, 19)); 
    m_sprite.setOrigin(16.0f / 2.0f, 10.0f);

}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(m_sprite);
}

sf::Vector2f Player::getPosition() const {
    return m_sprite.getPosition();
}