#include "Item.hpp"
#include <iostream>

Item::Item(Physics& physics, float x, float y)
: m_physics(physics), m_collected(false), m_spawning(true), m_spawnY(y), m_targetY(y - 32.0f), m_blinkTimer(0.0f), m_visible(true)
{
    if (!m_texture.loadFromFile("assets/images/items.png")) {
        std::cerr << "Error loading items.png" << std::endl;
    }
    m_sprite.setTexture(m_texture);
    
    // Red Mushroom
    // 18x16 sprite. Origin (9, 11) raises sprite 2px above 'perfect' alignment to ensure it sits visibly ON top of floor.
    m_sprite.setTextureRect(sf::IntRect(0, 0, 18, 16));
    m_sprite.setOrigin(9, 11);
    m_sprite.setScale(2.0f, 2.0f);
    m_sprite.setPosition(x, y); // Start inside block

    // Physics Body (Dynamic but kinematic while spawning)
    // For now, no physics body while spawning. We create it after spawn.
}

void Item::update(float dt) {
    // Blink only during spawn
    if (m_spawning) {
        m_blinkTimer += dt;
        if (m_blinkTimer > 0.1f) {
            m_blinkTimer = 0.0f;
            m_visible = !m_visible;
            
            // Toggle Alpha
            sf::Color c = m_sprite.getColor();
            c.a = m_visible ? 255 : 0;
            m_sprite.setColor(c);
        }
    }

    if (m_spawning) {
        // Move Up
        m_sprite.move(0, -30.0f * dt);
        if (m_sprite.getPosition().y <= m_targetY) {
            m_spawning = false;
            
            // Stop blinking - make fully visible
            sf::Color c = m_sprite.getColor();
            c.a = 255;
            m_sprite.setColor(c);
            
            // Create Physics Body now
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_dynamicBody;
            // Slightly adjust spawn Y up to ensure no deep overlap with ground
            bodyDef.position = (b2Vec2){m_sprite.getPosition().x / Physics::SCALE, (m_sprite.getPosition().y - 2.0f) / Physics::SCALE};
            bodyDef.fixedRotation = true;

            m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);
            
            // Slightly smaller physics box to endure it doesn't snag easily
            b2Polygon box = b2MakeBox((14.0f / 2.0f) / Physics::SCALE, (14.0f / 2.0f) / Physics::SCALE);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.friction = 0.0f; // Friction 0 to slide, or small value
            shapeDef.restitution = 0.0f;
            
            b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
            
            // Initial Push Right
            b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){2.0f, 0.0f});
        }
    } else {
        // Sync with Physics
        if (b2Body_IsValid(m_bodyId)) {
            b2Vec2 pos = b2Body_GetPosition(m_bodyId);
            m_sprite.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
            
            // Maintain horizontal velocity
            b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
            if (std::abs(vel.x) < 1.0f) {
                b2Body_SetLinearVelocity(m_bodyId, (b2Vec2){2.0f, vel.y});
            }
        }
    }
}

void Item::draw(sf::RenderWindow& window) {
    if (!m_collected) {
        window.draw(m_sprite);
    }
}

void Item::collect() {
    m_collected = true;
    if (b2Body_IsValid(m_bodyId)) {
        b2DestroyBody(m_bodyId);
        m_bodyId = b2_nullBodyId; 
    }
}
