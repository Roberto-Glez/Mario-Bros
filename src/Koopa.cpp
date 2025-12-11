#include "Koopa.hpp"
#include <iostream>
#include <cmath>

Koopa::Koopa(Physics& physics, float x, float y)
    : Enemy(physics, x, y)
    , m_koopaState(KoopaState::Walking)
    , m_shellFrame(0)
    , m_shellAnimTimer(0.0f)
{
    // Load texture (same as Goomba)
    if (!m_texture.loadFromFile("assets/images/Goomba_koopa.png")) {
        std::cerr << "Error loading Goomba_koopa.png" << std::endl;
    }
    m_sprite.setTexture(m_texture);
    
    // Set initial frame
    m_sprite.setTextureRect(sf::IntRect(SPRITE_OFFSET_X, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
    m_sprite.setOrigin(SPRITE_WIDTH / 2.0f, SPRITE_HEIGHT);
    m_sprite.setScale(2.0f, 2.0f);
    m_sprite.setPosition(x, y);
    
    // Create physics body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){x / Physics::SCALE, y / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);
    
    // Create hitbox (taller for Koopa)
    b2Polygon box = b2MakeOffsetBox(
        (14.0f / 2.0f) / Physics::SCALE, 
        (24.0f / 2.0f) / Physics::SCALE,
        (b2Vec2){0.0f, -(24.0f / 2.0f) / Physics::SCALE},
        0.0f
    );
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.friction = 0.0f;
    shapeDef.restitution = 0.0f;
    
    b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
    
    // Set initial velocity
    b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){WALK_SPEED * m_direction, 0.0f});
}

void Koopa::update(float dt) {
    if (m_state == State::Dead) {
        return;
    }
    
    // Handle shell state (doesn't use base Enemy update)
    if (m_koopaState == KoopaState::Shell) {
        // Just sit there with static sprite 5 (index 4) - no animation
        // Sync position only
        if (b2Body_IsValid(m_bodyId)) {
            b2Vec2 pos = b2Body_GetPosition(m_bodyId);
            m_sprite.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
        }
        return;
    }
    
    if (m_koopaState == KoopaState::ShellMoving) {
        // Shell sliding - animate through sprites 3-5 (indices 2, 3, 4)
        m_shellAnimTimer += dt;
        if (m_shellAnimTimer >= 0.05f) {
            m_shellAnimTimer = 0.0f;
            m_shellFrame = (m_shellFrame + 1) % 3;  // 3 frames: indices 2, 3, 4
            int shellX = SPRITE_OFFSET_X + (2 + m_shellFrame) * (SPRITE_WIDTH + SPRITE_GAP);
            m_sprite.setTextureRect(sf::IntRect(shellX, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
        }
        
        if (b2Body_IsValid(m_bodyId)) {
            b2Vec2 pos = b2Body_GetPosition(m_bodyId);
            m_sprite.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
            
            // Check wall collision to reverse
            b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
            if (std::abs(vel.x) < 0.1f) {
                m_direction *= -1.0f;
            }
            b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){SHELL_SPEED * m_direction, vel.y});
        }
        return;
    }
    
    // Walking state - use base class logic
    Enemy::update(dt);
}

void Koopa::stomp() {
    if (m_koopaState == KoopaState::Walking) {
        // First stomp: become shell
        m_koopaState = KoopaState::Shell;
        m_state = State::Stomped;  // This prevents base Enemy from dying
        
        // Set shell sprite (sprite 5 = index 4 is idle shell)
        int shellX = SPRITE_OFFSET_X + 4 * (SPRITE_WIDTH + SPRITE_GAP);
        m_sprite.setTextureRect(sf::IntRect(shellX, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
        
        // Stop movement
        if (b2Body_IsValid(m_bodyId)) {
            b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){0.0f, 0.0f});
        }
    } else if (m_koopaState == KoopaState::Shell) {
        // Second stomp: kick shell
        m_koopaState = KoopaState::ShellMoving;
        m_direction = 1.0f;  // Kick to the right
        
        if (b2Body_IsValid(m_bodyId)) {
            b2Body_SetType(m_bodyId, b2_dynamicBody);
            b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){SHELL_SPEED * m_direction, 0.0f});
        }
    } else if (m_koopaState == KoopaState::ShellMoving) {
        // Stomp moving shell: stop it and set to idle sprite
        m_koopaState = KoopaState::Shell;
        
        // Reset to idle shell sprite (index 4)
        int shellX = SPRITE_OFFSET_X + 4 * (SPRITE_WIDTH + SPRITE_GAP);
        m_sprite.setTextureRect(sf::IntRect(shellX, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
        
        if (b2Body_IsValid(m_bodyId)) {
            b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){0.0f, 0.0f});
        }
    }
}

void Koopa::updateAnimation(float dt) {
    if (m_koopaState != KoopaState::Walking) {
        return;
    }
    
    m_animationTimer += dt;
    if (m_animationTimer >= ANIMATION_SPEED) {
        m_animationTimer = 0.0f;
        m_currentFrame = (m_currentFrame + 1) % 2;
        
        int frameX = SPRITE_OFFSET_X + m_currentFrame * (SPRITE_WIDTH + SPRITE_GAP);
        m_sprite.setTextureRect(sf::IntRect(frameX, SPRITE_OFFSET_Y, SPRITE_WIDTH, SPRITE_HEIGHT));
    }
}

void Koopa::onStomp() {
    // Not used for Koopa - we override stomp() directly
}
