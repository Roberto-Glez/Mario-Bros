#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Player {
public:
    Player(Physics& physics, float startX, float startY);
    void handleInput(float dt); // Added dt for acceleration timer
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void grow();
    bool isBig() const { return m_isBig; }
    // Método helper para la cámara
    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;

private:
    void updateAnimation(float dt);

    Physics& m_physics;
    b2BodyId m_bodyId;
    
    sf::Texture m_texture;
    sf::Texture m_bigTexture;
    sf::Sprite m_sprite;
    
    float m_width;
    float m_height;
    bool m_canJump;
    bool m_isBig;

    // Animation state
    float m_animationTimer;
    float m_groundTimer; // To filter jump apex
    float m_runTimer;    // Momentum timer
    int m_currentFrame;
    bool m_facingRight;
    enum class State { Idle, Running, Jumping, Braking, Crouching } m_state;
};

#endif // PLAYER_HPP