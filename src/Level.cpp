#include "Level.hpp"

Level::Level(Physics& physics, float width, float height)
: m_physics(physics), m_width(width), m_height(height)
{
    // Ground rectangle at bottom
    m_groundY = height - 100.0f;
    m_groundShape.setSize(sf::Vector2f(width, 100.0f));
    m_groundShape.setFillColor(sf::Color(150, 75, 0));
    m_groundShape.setPosition(0.0f, m_groundY);

    // Create Box2D static body for ground
    b2BodyDef groundDef;
    groundDef.position.Set((width / 2.0f) / Physics::SCALE, (m_groundY + 50.0f) / Physics::SCALE);
    groundDef.type = b2_staticBody;
    m_groundBody = m_physics.world().CreateBody(&groundDef);

    b2PolygonShape groundBox;
    groundBox.SetAsBox((width / 2.0f) / Physics::SCALE, (50.0f) / Physics::SCALE);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &groundBox;
    fixtureDef.friction = 0.6f;
    m_groundBody->CreateFixture(&fixtureDef);
}

void Level::draw(sf::RenderWindow& window)
{
    window.draw(m_groundShape);
}

float Level::groundY() const
{
    return m_groundY;
}
