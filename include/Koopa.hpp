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
    bool isIdleShell() const { return m_koopaState == KoopaState::Shell; }
    void kick(float direction);  // Kick shell in a direction (-1 left, 1 right)

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
    static constexpr int SPRITE_GAP = 7;  // Gap between sprites
    
    // Shell sprite positions (exact pixel coordinates)
    static constexpr int SHELL_SPRITE_3_X = 170;
    static constexpr int SHELL_SPRITE_3_Y = 11;
    static constexpr int SHELL_SPRITE_4_X = 195;
    static constexpr int SHELL_SPRITE_4_Y = 12;
    static constexpr int SHELL_WIDTH = 17;
    static constexpr int SHELL_HEIGHT = 16;  // Shell is shorter than walking Koopa
    
    static constexpr float SHELL_SPEED = 5.0f;
};

#endif // KOOPA_HPP
