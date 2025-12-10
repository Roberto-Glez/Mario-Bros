#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <SFML/Graphics.hpp>
#include "Physics.hpp"

class Level {
public:
    Level(Physics& physics, float width, float height);
    void draw(sf::RenderWindow& window);
    float groundY() const; 

private:
    Physics& m_physics;
    
    sf::VertexArray m_groundVertices;
    sf::Texture m_texture;
    
    static constexpr int TILE_SIZE = 16;

    b2BodyId m_groundBodyId;
    float m_width;
    float m_height;
    float m_groundY;
};

#endif // LEVEL_HPP