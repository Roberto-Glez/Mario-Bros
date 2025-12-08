#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath> // Para std::abs

Player::Player(Physics& physics, float startX, float startY)
: m_physics(physics), m_width(32.0f), m_height(48.0f), m_canJump(false)
{
    m_shape.setSize(sf::Vector2f(m_width, m_height));
    m_shape.setOrigin(m_width / 2.0f, m_height / 2.0f);
    m_shape.setFillColor(sf::Color::Red);

    // Definición del cuerpo (v3)
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){startX / Physics::SCALE, startY / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    // Crear el cuerpo usando el ID del mundo
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

    // Definir la forma (Caja)
    b2Polygon dynamicBox = b2MakeBox((m_width / 2.0f) / Physics::SCALE, (m_height / 2.0f) / Physics::SCALE);

    // Definir la "fixture" (propiedades físicas)
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;

    // Unir forma al cuerpo
    b2CreatePolygonShape(m_bodyId, &shapeDef, &dynamicBox);
}

void Player::handleInput()
{
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
    float desiredVel = 0.0f;
    const float moveSpeed = 4.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        desiredVel = -moveSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        desiredVel = moveSpeed;
    }

    // Cambio de velocidad instantáneo (simple para plataformas)
    // En v3 asignamos velocidad directamente o usamos impulsos.
    // Para mantener el estilo del código anterior usando impulsos:
    float velChange = desiredVel - vel.x;
    float impulse = b2Body_GetMass(m_bodyId) * velChange;
    
    b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){impulse, 0.0f}, true);

    // Salto
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) && m_canJump) {
        float jumpImpulse = -b2Body_GetMass(m_bodyId) * 6.0f;
        b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){0.0f, jumpImpulse}, true);
        m_canJump = false;
    }
}

void Player::update(float dt)
{
    b2Vec2 pos = b2Body_GetPosition(m_bodyId);
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

    // Actualizar gráfico SFML
    m_shape.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);

    // Chequeo simple de suelo
    if (std::abs(vel.y) < 0.1f) m_canJump = true;
    else m_canJump = false;
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(m_shape);
}

sf::Vector2f Player::getPosition() const {
    return m_shape.getPosition();
}