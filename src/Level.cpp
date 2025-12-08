#include "Level.hpp"

Level::Level(Physics& physics, float width, float height)
: m_physics(physics), m_width(width), m_height(height)
{
    // Gráfico del suelo
    m_groundY = height - 100.0f;
    m_groundShape.setSize(sf::Vector2f(width, 100.0f));
    m_groundShape.setFillColor(sf::Color(150, 75, 0));
    m_groundShape.setPosition(0.0f, m_groundY);

    // Cuerpo estático Box2D v3
    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = (b2Vec2){(width / 2.0f) / Physics::SCALE, (m_groundY + 50.0f) / Physics::SCALE};
    // Por defecto es estático, no hace falta especificar b2_staticBody explícitamente si usamos el default, 
    // pero para estar seguros:
    groundDef.type = b2_staticBody; 

    m_groundBodyId = b2CreateBody(m_physics.worldId(), &groundDef);

    // Forma
    b2Polygon groundBox = b2MakeBox((width / 2.0f) / Physics::SCALE, (50.0f) / Physics::SCALE);

    // Fixture
    b2ShapeDef fixtureDef = b2DefaultShapeDef();
    fixtureDef.friction = 0.6f;
    
    b2CreatePolygonShape(m_groundBodyId, &fixtureDef, &groundBox);
}

void Level::draw(sf::RenderWindow& window)
{
    window.draw(m_groundShape);
}

float Level::groundY() const
{
    return m_groundY;
}