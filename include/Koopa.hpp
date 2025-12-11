#ifndef KOOPA_HPP
#define KOOPA_HPP

#include "Enemy.hpp"

class Koopa : public Enemy {
public:
    enum class KoopaState { Walking, Shell, ShellMoving };
    
    Koopa(Physics& physics, float x, float y);
    
    void update(float dt) override;
    void stomp() override;
    
    bool isShell() const { return m_koopaState == KoopaState::Shell || m_koopaState == KoopaState::ShellMoving; }

protected:
    void updateAnimation(float dt) override;
    void onStomp() override;

private:
    KoopaState m_koopaState;
    int m_shellFrame;
    float m_shellAnimTimer;
    
    // Sprite dimensions (from spritesheet)
    static constexpr int SPRITE_WIDTH = 17;
    static constexpr int SPRITE_HEIGHT = 28;
    static constexpr int SPRITE_OFFSET_X = 123;
    static constexpr int SPRITE_OFFSET_Y = 2;
    static constexpr int SPRITE_GAP = 5;  // Gap between sprites (adjusted for shell sprites)
    
    static constexpr float SHELL_SPEED = 5.0f;
};

#endif // KOOPA_HPP
