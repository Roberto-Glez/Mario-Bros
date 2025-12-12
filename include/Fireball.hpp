#ifndef FIREBALL_HPP
#define FIREBALL_HPP

#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Fireball {
public:
    Fireball(Physics& physics, float x, float y, float direction);
    ~Fireball();

    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    bool isAlive() const { return m_alive; }
    sf::FloatRect getBounds() const { return m_sprite.getGlobalBounds(); }
    sf::Vector2f getPosition() const;
    void destroy();

private:
    Physics& m_physics;
    b2BodyId m_bodyId;
    
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    
    float m_animTimer;
    int m_frame;
    bool m_alive;
    float m_direction;
    int m_bounceCount;
    
    static constexpr float SPEED = 8.0f;
    static constexpr float ANIMATION_SPEED = 0.05f;
    static constexpr int MAX_BOUNCES = 4;
    
    // Sprite coordinates in items.png
    static constexpr int SPRITE_SIZE = 8;
    static constexpr int FRAME_POSITIONS[4][2] = {
        {3, 58},   // Frame 0
        {21, 57},  // Frame 1
        {39, 58},  // Frame 2
        {57, 57}   // Frame 3
    };
};

#endif // FIREBALL_HPP
