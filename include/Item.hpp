#pragma once
#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Item {
public:
    Item(Physics& physics, float x, float y);
    virtual ~Item() = default;

    virtual void update(float dt);
    virtual void draw(sf::RenderWindow& window);
    sf::FloatRect getBounds() const { return m_sprite.getGlobalBounds(); }

    bool isCollected() const { return m_collected; }
    void collect();

protected:
    Physics& m_physics;
    b2BodyId m_bodyId;
    sf::Sprite m_sprite;
    sf::Texture m_texture;
    
    bool m_collected;
    bool m_spawning;
    float m_spawnY;
    float m_targetY;
    
    // Blink animation
    float m_blinkTimer;
    bool m_visible;
};
