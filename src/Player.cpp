#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>

Player::Player(Physics& physics, float startX, float startY)
: m_physics(physics), m_body(nullptr), m_width(32.0f), m_height(48.0f), m_canJump(false)
{
    m_shape.setSize(sf::Vector2f(m_width, m_height));
    m_shape.setOrigin(m_width / 2.0f, m_height / 2.0f);
    m_shape.setFillColor(sf::Color::Red);

    // Create Box2D body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(startX / Physics::SCALE, startY / Physics::SCALE);
    bodyDef.fixedRotation = true;
    m_body = m_physics.world().CreateBody(&bodyDef);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox((m_width / 2.0f) / Physics::SCALE, (m_height / 2.0f) / Physics::SCALE);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    m_body->CreateFixture(&fixtureDef);
}

void Player::handleInput()
{
    b2Vec2 vel = m_body->GetLinearVelocity();
    float desiredVel = 0.0f;
    const float moveSpeed = 4.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        desiredVel = -moveSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        desiredVel = moveSpeed;
    }

    float velChange = desiredVel - vel.x;
    float impulse = m_body->GetMass() * velChange; // p = m * dv
    m_body->ApplyLinearImpulseToCenter(b2Vec2(impulse, 0.0f), true);

    // Jump
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) && m_canJump) {
        m_body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, -m_body->GetMass() * 6.0f), true);
        m_canJump = false;
    }
}

void Player::update(float dt)
{
    b2Vec2 pos = m_body->GetPosition();
    b2Vec2 vel = m_body->GetLinearVelocity();

    // Update SFML shape
    m_shape.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);

    // Very simple ground-check: if vertical velocity is near zero, allow jump
    if (std::abs(vel.y) < 0.1f) m_canJump = true;
    else m_canJump = false;
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(m_shape);
}
