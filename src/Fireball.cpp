#include "Fireball.hpp"
#include <iostream>
#include <cmath>

Fireball::Fireball(Physics& physics, float x, float y, float direction)
    : m_physics(physics)
    , m_sprite(m_texture)
    , m_animTimer(0.0f)
    , m_frame(0)
    , m_alive(true)
    , m_direction(direction)
    , m_bounceCount(0)
{
    // Load texture
    if (!m_texture.loadFromFile("assets/images/items.png")) {
        std::cerr << "Error loading items.png for Fireball" << std::endl;
    }
    m_sprite.setTexture(m_texture);
    
    // Set initial frame
    m_sprite.setTextureRect(sf::IntRect({FRAME_POSITIONS[0][0], FRAME_POSITIONS[0][1]}, {SPRITE_SIZE, SPRITE_SIZE}));
    m_sprite.setOrigin({SPRITE_SIZE / 2.0f, SPRITE_SIZE / 2.0f});
    m_sprite.setScale({2.0f, 2.0f});
    m_sprite.setPosition({x, y});
    
    // Create physics body - kinematic to ignore gravity
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_kinematicBody; // Kinematic = no gravity, we control velocity
    bodyDef.position = (b2Vec2){x / Physics::SCALE, y / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);
    
    // Create small circle-like box for collision
    b2Polygon box = b2MakeBox((6.0f / 2.0f) / Physics::SCALE, (6.0f / 2.0f) / Physics::SCALE);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 0.5f;
    
    b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
    
    // Set constant horizontal velocity (straight line, no Y velocity)
    b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){SPEED * m_direction, 0.0f});
}

Fireball::~Fireball() {
    if (b2Body_IsValid(m_bodyId)) {
        b2DestroyBody(m_bodyId);
    }
}

void Fireball::update(float dt) {
    if (!m_alive) return;
    
    // Animation
    m_animTimer += dt;
    if (m_animTimer >= ANIMATION_SPEED) {
        m_animTimer = 0.0f;
        m_frame = (m_frame + 1) % 4;
        m_sprite.setTextureRect(sf::IntRect(
            {FRAME_POSITIONS[m_frame][0], FRAME_POSITIONS[m_frame][1]}, 
            {SPRITE_SIZE, SPRITE_SIZE}
        ));
    }
    
    // Sync with physics
    if (b2Body_IsValid(m_bodyId)) {
        b2Vec2 pos = b2Body_GetPosition(m_bodyId);
        
        m_sprite.setPosition({pos.x * Physics::SCALE, pos.y * Physics::SCALE});
        
        // Keep constant horizontal velocity (straight line)
        b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){SPEED * m_direction, 0.0f});
        
        // Destroy if out of bounds
        if (pos.x * Physics::SCALE < -100.0f || pos.x * Physics::SCALE > 3500.0f) {
            destroy();
        }
    }
}

void Fireball::draw(sf::RenderWindow& window) {
    if (m_alive) {
        window.draw(m_sprite);
    }
}

sf::Vector2f Fireball::getPosition() const {
    if (b2Body_IsValid(m_bodyId)) {
        b2Vec2 pos = b2Body_GetPosition(m_bodyId);
        return sf::Vector2f(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
    }
    return m_sprite.getPosition();
}

void Fireball::destroy() {
    m_alive = false;
    if (b2Body_IsValid(m_bodyId)) {
        b2DestroyBody(m_bodyId);
        m_bodyId = b2_nullBodyId;
    }
}
