#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Enemy {
public:
    enum class State { Walking, Stomped, Dead };

    Enemy(Physics& physics, float x, float y);
    virtual ~Enemy();

    virtual void update(float dt);
    virtual void draw(sf::RenderWindow& window);
    virtual void stomp();  // Called when Mario jumps on the enemy
    
    bool isAlive() const { return m_state != State::Dead; }
    bool isStomped() const { return m_state == State::Stomped; }
    sf::FloatRect getBounds() const { return m_sprite.getGlobalBounds(); }
    sf::Vector2f getPosition() const;

protected:
    virtual void updateAnimation(float dt) = 0;
    virtual void onStomp() = 0;  // Override for specific stomp behavior

    Physics& m_physics;
    b2BodyId m_bodyId;
    
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    
    State m_state;
    
    // Animation
    float m_animationTimer;
    int m_currentFrame;
    
    // Movement
    float m_direction;  // 1.0 for right, -1.0 for left
    
    // Death timer (after stomp)
    float m_stompTimer;
    static constexpr float STOMP_DELAY = 0.5f;
    
    // Common constants
    static constexpr float WALK_SPEED = 1.5f;
    static constexpr float ANIMATION_SPEED = 0.15f;
};

#endif // ENEMY_HPP
