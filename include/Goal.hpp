#ifndef GOAL_HPP
#define GOAL_HPP

#include <SFML/Graphics.hpp>

class Goal {
public:
  Goal();
  
  void init(float x, float y);
  void update(float dt);
  void draw(sf::RenderWindow &window);
  
  void trigger(); // Called when Mario reaches the goal
  bool isTriggered() const { return m_triggered; }
  bool isAnimationComplete() const { return m_animComplete; }
  
  sf::FloatRect getBounds() const;
  float getX() const { return m_x; }

private:
  sf::Texture m_texture;
  sf::Sprite m_sprite;
  
  float m_x;
  float m_y;
  
  bool m_triggered;
  bool m_animComplete;
  
  // Animation
  float m_animTimer;
  int m_frame;
  static constexpr int NUM_FRAMES = 4; // Animation frames 1-4
  static constexpr float FRAME_TIME = 0.1f;
  static constexpr float ANIM_DURATION = 2.0f; // Total animation time
  float m_totalAnimTime;
  
  // Animation sprite dimensions (flag)
  static constexpr int SPRITE_X = 2;
  static constexpr int SPRITE_Y = 6;
  static constexpr int SPRITE_WIDTH = 47;
  static constexpr int SPRITE_HEIGHT = 49;
  static constexpr int SPRITE_GAP = 2; // Gap between frames
  
  // Pole sprite (static, always drawn)
  static constexpr int POLE_X = 98;
  static constexpr int POLE_Y = 7;
  static constexpr int POLE_WIDTH = 8;
  static constexpr int POLE_HEIGHT = 48;
  
  sf::Sprite m_poleSprite;
};

#endif // GOAL_HPP
