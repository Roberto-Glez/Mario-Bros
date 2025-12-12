#ifndef KOOPA_HPP
#define KOOPA_HPP

#include "Enemy.hpp"

class Koopa : public Enemy {
public:
    enum class KoopaState { Walking, Shell, ShellMoving, ShellDying };
    
    Koopa(Physics& physics, float x, float y);
    
    void update(float dt) override;
    void stomp() override;
    
    bool isShell() const { return m_koopaState == KoopaState::Shell || m_koopaState == KoopaState::ShellMoving; }
    bool isIdleShell() const { return m_koopaState == KoopaState::Shell; }
    void kick(float direction, float kickerAbsVelocityX = 0.0f);  // Kick shell in a direction (-1 left, 1 right)
    void killByFireball();  // Kill shell with fireball - jump and flip animation

protected:
    void updateAnimation(float dt) override;
    void onStomp() override;

private:
    KoopaState m_koopaState;
    int m_shellFrame;
    float m_shellAnimTimer;
    
    // Sprite dimensions (from spritesheet)
    // Sprite dimensions (from spritesheet)
    static constexpr int SPRITE_WIDTH = 21;
    static constexpr int SPRITE_HEIGHT = 32;
    static constexpr int SPRITE_OFFSET_X = 121;
    static constexpr int SPRITE_OFFSET_Y = 0;
    static constexpr int SPRITE_GAP = 3;  // Gap between sprites
    
    // Shell sprite positions (exact pixel coordinates)
    // Shell sprite positions (exact pixel coordinates)
    static constexpr int SHELL_SPRITE_3_X = 169;
    static constexpr int SHELL_SPRITE_3_Y = 8;
    static constexpr int SHELL_SPRITE_4_X = 194;
    static constexpr int SHELL_SPRITE_4_Y = 9;
    static constexpr int SHELL_WIDTH = 23;
    static constexpr int SHELL_HEIGHT = 26;  // Shell is shorter than walking Koopa
    
    static constexpr float SHELL_SPEED = 5.0f;
};

#endif // KOOPA_HPP
