#include "Enemy.hpp"
#include <iostream>
#include <cmath>

Enemy::Enemy(Physics& physics, float x, float y)
    : m_physics(physics)
    , m_state(State::Walking)
    , m_animationTimer(0.0f)
    , m_currentFrame(0)
    , m_direction(-1.0f)  // Start walking left
    , m_stompTimer(0.0f)
{
}

Enemy::~Enemy() {
    if (b2Body_IsValid(m_bodyId)) {
        b2DestroyBody(m_bodyId);
    }
}

void Enemy::update(float dt) {
    if (m_state == State::Dead) {
        return;
    }
    
    if (m_state == State::Stomped) {
        m_stompTimer += dt;
        if (m_stompTimer >= STOMP_DELAY) {
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
        
        // Flip sprite based on direction
        if (m_direction > 0) {
            m_sprite.setScale(-2.0f, 2.0f);  // Facing right (mirrored)
        } else {
            m_sprite.setScale(2.0f, 2.0f);   // Facing left (normal)
        }
        
        // Maintain horizontal velocity
        b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
        
        // Check if we hit a wall
        if (std::abs(vel.x) < 0.1f) {
            m_direction *= -1.0f;
        }
        
        // Keep moving
        b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){WALK_SPEED * m_direction, vel.y});
    }
}

void Enemy::draw(sf::RenderWindow& window) {
    if (m_state != State::Dead) {
        window.draw(m_sprite);
    }
}

void Enemy::stomp() {
    if (m_state != State::Walking) {
        return;
    }
    
    m_state = State::Stomped;
    m_stompTimer = 0.0f;
    
    // Call specific stomp behavior
    onStomp();
    
    // Stop physics body
    if (b2Body_IsValid(m_bodyId)) {
        b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){0.0f, 0.0f});
        b2Body_SetType(m_bodyId, b2_kinematicBody);
    }
}

sf::Vector2f Enemy::getPosition() const {
    if (b2Body_IsValid(m_bodyId)) {
        b2Vec2 pos = b2Body_GetPosition(m_bodyId);
        return sf::Vector2f(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
    }
    return m_sprite.getPosition();
}
