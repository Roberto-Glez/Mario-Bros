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
    Physics& m_physics;
    b2BodyId m_bodyId; // Cambio importante
    sf::RectangleShape m_shape; // O Sprite si ya lo cambiaste
    float m_width;
    float m_height;
    bool m_canJump;
};

#endif // PLAYER_HPP