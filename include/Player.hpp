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

private:
    Physics& m_physics;
    b2Body* m_body;
    sf::RectangleShape m_shape;
    float m_width;
    float m_height;
    bool m_canJump;
};

#endif // PLAYER_HPP
