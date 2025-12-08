#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <box2d/box2d.h>

class Physics {
public:
    static constexpr float SCALE = 30.0f; // pixels per meter

    Physics();
    ~Physics();

    void step(float dt);
    b2World& world();

private:
    b2World m_world;
};

#endif // PHYSICS_HPP
