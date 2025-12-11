#include "Goomba.hpp"
#include <iostream>
#include <cmath>

Goomba::Goomba(Physics& physics, float x, float y)
    : m_physics(physics)
    , m_state(State::Walking)
    , m_animationTimer(0.0f)
    , m_currentFrame(0)
    , m_direction(-1.0f)  // Start walking left (like original game)
    , m_deathTimer(0.0f)
{
    // Load texture
    if (!m_texture.loadFromFile("assets/images/Goomba_koopa.png")) {
        std::cerr << "Error loading Goomba_koopa.png" << std::endl;
    }
    m_sprite.setTexture(m_texture);
    
    // Set initial frame (first walking frame)
    // Sprite starts at (2, 12) with size 17x17
    m_sprite.setTextureRect(sf::IntRect(SPRITE_OFFSET_X, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
    // Origin at bottom-center so sprite sits on ground properly
    m_sprite.setOrigin(SPRITE_WIDTH / 2.0f, SPRITE_HEIGHT);
    m_sprite.setScale(2.0f, 2.0f);  // Scale to match game size
    m_sprite.setPosition(x, y);
    
    // Create physics body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){x / Physics::SCALE, y / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);
    
    // Create hitbox - positioned at the bottom center of the sprite
    // Physics body center is at the origin point (bottom of sprite)
    // So we offset the box up by half its height
    b2Polygon box = b2MakeOffsetBox(
        (14.0f / 2.0f) / Physics::SCALE, 
        (14.0f / 2.0f) / Physics::SCALE,
        (b2Vec2){0.0f, -(14.0f / 2.0f) / Physics::SCALE},  // Offset up
        0.0f  // angle
    );
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.friction = 0.0f;
    shapeDef.restitution = 0.0f;
    
    b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
    
    // Set initial velocity
    b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){WALK_SPEED * m_direction, 0.0f});
}

Goomba::~Goomba() {
    if (b2Body_IsValid(m_bodyId)) {
        b2DestroyBody(m_bodyId);
    }
}

void Goomba::update(float dt) {
    if (m_state == State::Dead) {
        return;
    }
    
    if (m_state == State::Squashed) {
        m_deathTimer += dt;
        if (m_deathTimer >= DEATH_DELAY) {
            m_state = State::Dead;
            if (b2Body_IsValid(m_bodyId)) {
                b2DestroyBody(m_bodyId);
                m_bodyId = b2_nullBodyId;
            }
        }
        return;
    }
    
    // Walking state
    updateAnimation(dt);
    
    // Sync sprite with physics
    if (b2Body_IsValid(m_bodyId)) {
        b2Vec2 pos = b2Body_GetPosition(m_bodyId);
        m_sprite.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
        
        // Flip sprite based on direction (like Mario does)
        if (m_direction > 0) {
            m_sprite.setScale(-2.0f, 2.0f);  // Facing right (mirrored)
        } else {
            m_sprite.setScale(2.0f, 2.0f);   // Facing left (normal)
        }
        
        // Maintain horizontal velocity
        b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
        
        // Check if we hit a wall (velocity became 0 or changed sign)
        if (std::abs(vel.x) < 0.1f) {
            m_direction *= -1.0f;  // Reverse direction
        }
        
        // Keep moving
        b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){WALK_SPEED * m_direction, vel.y});
    }
}

void Goomba::updateAnimation(float dt) {
    if (m_state != State::Walking) {
        return;
    }
    
    m_animationTimer += dt;
    if (m_animationTimer >= ANIMATION_SPEED) {
        m_animationTimer = 0.0f;
        m_currentFrame = (m_currentFrame + 1) % 2;  // Alternate between 0 and 1
        
        // Update texture rect - calculate X position for current frame
        int frameX = SPRITE_OFFSET_X + m_currentFrame * (SPRITE_WIDTH + SPRITE_GAP);
        m_sprite.setTextureRect(sf::IntRect(frameX, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
    }
}

void Goomba::draw(sf::RenderWindow& window) {
    if (m_state != State::Dead) {
        window.draw(m_sprite);
    }
}

void Goomba::stomp() {
    if (m_state != State::Walking) {
        return;
    }
    
    m_state = State::Squashed;
    m_deathTimer = 0.0f;
    
    // Set squashed sprite (frame 3 = index 2) - needs extra offset to avoid cropping
    int squashedX = SPRITE_OFFSET_X + 2 * (SPRITE_WIDTH + SPRITE_GAP) + 4;  // +4 extra for third sprite
    m_sprite.setTextureRect(sf::IntRect(squashedX, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
    
    // Stop physics body
    if (b2Body_IsValid(m_bodyId)) {
        b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){0.0f, 0.0f});
        // Make it kinematic so it doesn't move
        b2Body_SetType(m_bodyId, b2_kinematicBody);
    }
}

sf::Vector2f Goomba::getPosition() const {
    if (b2Body_IsValid(m_bodyId)) {
        b2Vec2 pos = b2Body_GetPosition(m_bodyId);
        return sf::Vector2f(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
    }
    return m_sprite.getPosition();
}
