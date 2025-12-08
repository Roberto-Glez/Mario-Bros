#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Level {
public:
    Level(Physics& physics, float width, float height);
    void draw(sf::RenderWindow& window);
    float groundY() const; // in pixels
private:
    Physics& m_physics;
    sf::RectangleShape m_groundShape;
    b2Body* m_groundBody;
    float m_width;
    float m_height;
    float m_groundY;
};

#endif // LEVEL_HPP
