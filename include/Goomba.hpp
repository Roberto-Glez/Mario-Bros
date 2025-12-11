#ifndef GOOMBA_HPP
#define GOOMBA_HPP

#include "Enemy.hpp"

class Goomba : public Enemy {
public:
    Goomba(Physics& physics, float x, float y);

protected:
    void updateAnimation(float dt) override;
    void onStomp() override;

private:
    // Sprite dimensions (from spritesheet)
    static constexpr int SPRITE_WIDTH = 17;
    static constexpr int SPRITE_HEIGHT = 17;
    static constexpr int SPRITE_OFFSET_X = 2;
    static constexpr int SPRITE_OFFSET_Y = 10;
    static constexpr int SPRITE_GAP = 5;
};

#endif // GOOMBA_HPP
