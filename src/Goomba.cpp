#include "Goomba.hpp"
#include <iostream>

Goomba::Goomba(Physics& physics, float x, float y)
    : Enemy(physics, x, y)
{
    // Load texture
    if (!m_texture.loadFromFile("assets/images/Goomba_koopa.png")) {
        std::cerr << "Error loading Goomba_koopa.png" << std::endl;
    }
    m_sprite.setTexture(m_texture);
    
    // Set initial frame
    m_sprite.setTextureRect(sf::IntRect({SPRITE_OFFSET_X, SPRITE_OFFSET_Y}, {SPRITE_WIDTH, SPRITE_HEIGHT}));
    m_sprite.setOrigin({SPRITE_WIDTH / 2.0f, SPRITE_HEIGHT});
    m_sprite.setScale({2.0f, 2.0f});
    m_sprite.setPosition({x, y});
    
    // Create physics body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){x / Physics::SCALE, y / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);
    
    // Create hitbox (Goomba 17x9)
    b2Polygon box = b2MakeOffsetBox(
        (17.0f / 2.0f) / Physics::SCALE, 
        (9.0f / 2.0f) / Physics::SCALE,
        (b2Vec2){0.0f, -(9.0f / 2.0f) / Physics::SCALE},
        b2MakeRot(0.0f)
    );
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    // shapeDef.friction = 0.0f;
    // shapeDef.restitution = 0.0f;
    
    b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
    
    // Set initial velocity
    b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){WALK_SPEED * m_direction, 0.0f});
}

void Goomba::updateAnimation(float dt) {
    m_animationTimer += dt;
    if (m_animationTimer >= ANIMATION_SPEED) {
        m_animationTimer = 0.0f;
        m_currentFrame = (m_currentFrame + 1) % 2;
        
        int frameX = SPRITE_OFFSET_X + m_currentFrame * (SPRITE_WIDTH + SPRITE_GAP);
        m_sprite.setTextureRect(sf::IntRect({frameX, SPRITE_OFFSET_Y}, {SPRITE_WIDTH, SPRITE_HEIGHT}));
    }
}

void Goomba::onStomp() {
    // Set squashed sprite (frame 3 = index 2)
    int squashedX = SPRITE_OFFSET_X + 2 * (SPRITE_WIDTH + SPRITE_GAP) + 4;
    m_sprite.setTextureRect(sf::IntRect({squashedX, SPRITE_OFFSET_Y}, {SPRITE_WIDTH, SPRITE_HEIGHT}));
}
