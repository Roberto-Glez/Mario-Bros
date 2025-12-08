#include "Physics.hpp"

Physics::Physics()
{
    // En v3 se usa una estructura de definición y una función de creación
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, 9.8f}; // Llaves para struct C
    m_worldId = b2CreateWorld(&worldDef);
}

Physics::~Physics() {
    b2DestroyWorld(m_worldId);
}

void Physics::step(float dt)
{
    // El paso de física en v3 es más simple
    b2World_Step(m_worldId, dt, 4); // 4 sub-steps es un buen default
}

b2WorldId Physics::worldId()
{
    return m_worldId;
}