#pragma once
#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Block {
public:
    enum class Type { Question, Empty };

    Block(Physics& physics, float x, float y);
    bool hit();  // Returns true if item should spawn (first hit only)
    void draw(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    bool isActive() const { return m_active; }
    void update(float dt); // For animations if needed

    // Position helper for spawning items
    sf::Vector2f getPosition() const;

private:
    void updateTexture();

    Physics& m_physics;
    b2BodyId m_bodyId;
    sf::Texture m_texture; // We might want to clear this if sharing textures, but for now individual texture
    sf::Sprite m_sprite;
    
    Type m_type;
    bool m_active;
    
    // Animation state
    float m_animTimer;
    int m_frame;
};
