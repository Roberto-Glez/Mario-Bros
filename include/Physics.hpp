#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <box2d/box2d.h>

class Physics {
public:
    static constexpr float SCALE = 30.0f;

    Physics();
    ~Physics();

    void step(float dt);
    b2WorldId worldId(); // Cambio de referencia a ID

private:
    b2WorldId m_worldId;
};

#endif // PHYSICS_HPP
