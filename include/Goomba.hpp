#ifndef GOOMBA_HPP
#define GOOMBA_HPP

#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Goomba {
public:
    enum class State { Walking, Squashed, Dead };

    Goomba(Physics& physics, float x, float y);
    ~Goomba();

    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    void stomp();  // Called when Mario jumps on the Goomba
    
    bool isAlive() const { return m_state != State::Dead; }
    bool isSquashed() const { return m_state == State::Squashed; }
    sf::FloatRect getBounds() const { return m_sprite.getGlobalBounds(); }
    sf::Vector2f getPosition() const;

private:
    void updateAnimation(float dt);

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
    
    // Death timer (after squash)
    float m_deathTimer;
    static constexpr float DEATH_DELAY = 0.5f;
    
    // Sprite dimensions (from spritesheet)
    static constexpr int SPRITE_WIDTH = 17;
    static constexpr int SPRITE_HEIGHT = 17;
    static constexpr int SPRITE_OFFSET_X = 2;   // First sprite starts at x=2
    static constexpr int SPRITE_OFFSET_Y = 10;  // First sprite starts at y=10 (adjusted -2 to sit on ground)
    static constexpr int SPRITE_GAP = 5;        // Gap between sprites (increased to avoid right-side cropping)
    static constexpr float WALK_SPEED = 1.5f;
    static constexpr float ANIMATION_SPEED = 0.15f;
};

#endif // GOOMBA_HPP
