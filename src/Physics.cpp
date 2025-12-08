#include "Physics.hpp"

Physics::Physics()
: m_world(b2Vec2(0.0f, 9.8f))
{
}

Physics::~Physics() {}

void Physics::step(float dt)
{
    // Fixed-step for Box2D
    const int32 velocityIterations = 8;
    const int32 positionIterations = 3;
    m_world.Step(dt, velocityIterations, positionIterations);
}

b2World& Physics::world()
{
    return m_world;
}
