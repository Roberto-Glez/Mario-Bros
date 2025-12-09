#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Player {
public:
    Player(Physics& physics, float startX, float startY);
    void handleInput();
    void update(float dt);
    void draw(sf::RenderWindow& window);
    // Método helper para la cámara
    sf::Vector2f getPosition() const;

private:
    void updateAnimation(float dt);

    Physics& m_physics;
    b2BodyId m_bodyId;
    
    sf::Sprite m_sprite;
    sf::Texture m_texture;

    // Animation state
    enum class AnimationState {
        Idle,
        Run,
        Jump,
        Landing
    };
    
    AnimationState m_currentState;
    float m_animationTimer;
    int m_currentFrame;
    bool m_facingRight;

    float m_width;
    float m_height;
    bool m_canJump;
};

#endif // PLAYER_HPP