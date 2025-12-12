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
    
    static constexpr int SPRITE_WIDTH = 21;
    static constexpr int SPRITE_HEIGHT = 23;
    static constexpr int SPRITE_OFFSET_X = 0;
    static constexpr int SPRITE_OFFSET_Y = 6;
    static constexpr int SPRITE_GAP = 1;
};

#endif // GOOMBA_HPP
