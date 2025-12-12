#include "FireFlower.hpp"
#include <iostream>

FireFlower::FireFlower(Physics& physics, float x, float y)
: Item(physics, x, y)
{
    // Fire Flower sprite at (0, 18) with size 18x18
    m_sprite.setTextureRect(sf::IntRect({0, 18}, {18, 18}));
    m_sprite.setOrigin({9.f, 11.f});  // Center horizontally, lowered to ground
    m_sprite.setScale({2.0f, 2.0f});
    m_sprite.setPosition({x, y});
}

void FireFlower::update(float dt) {
    // Blink during spawn (same as parent)
    if (m_spawning) {
        m_blinkTimer += dt;
        if (m_blinkTimer > 0.1f) {
            m_blinkTimer = 0.0f;
            m_visible = !m_visible;
            
            sf::Color c = m_sprite.getColor();
            c.a = m_visible ? 255 : 0;
            m_sprite.setColor(c);
        }
        
        // Move up out of block
        m_sprite.move({0.f, -30.0f * dt});
        if (m_sprite.getPosition().y <= m_targetY) {
            m_spawning = false;
            
            // Stop blinking
            sf::Color c = m_sprite.getColor();
            c.a = 255;
            m_sprite.setColor(c);
            
            // Fire Flower does NOT create physics body - it stays still
            // No horizontal movement unlike Mushroom
        }
    }
    // Fire Flower stays in place after spawning (no physics body, no movement)
}
