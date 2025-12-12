#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>
#include <memory>
#include "Physics.hpp"
#include "Block.hpp"
#include "Item.hpp"
#include "FireFlower.hpp"
#include "Enemy.hpp"
#include "Goomba.hpp"
#include "Koopa.hpp"

// Forward declaration
class Player;

class Level {
public:
    Level(Physics& physics, float width, float height);
    void draw(sf::RenderWindow& window);
    void update(float dt);
    void checkCollisions(Player& player);
    
    // Helper para la cámara
    float groundY() const;

private:
    Physics& m_physics;
    
    sf::VertexArray m_groundVertices;
    sf::Texture m_texture;
    
    std::vector<Block> m_blocks;
    std::vector<std::unique_ptr<Item>> m_items;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    
    static constexpr int TILE_SIZE = 16;
    
    b2BodyId m_groundBodyId;
    float m_width;
    float m_height;
    float m_groundY;
    
    // Stomp cooldown to prevent ghost bounces and stomp loops
    float m_stompCooldown;
    static constexpr float STOMP_COOLDOWN_TIME = 0.5f;  // 500ms cooldown
};

#endif // LEVEL_HPP