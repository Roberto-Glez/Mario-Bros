#include "Level.hpp"
#include "Player.hpp"
#include <iostream>
#include <algorithm>

Level::Level(Physics& physics, float width, float height)
: m_physics(physics), m_width(width), m_height(height), m_stompCooldown(0.0f)
{
    // Load Texture
    if (!m_texture.loadFromFile("assets/images/blocks.png")) {
        std::cerr << "Error loading blocks.png" << std::endl;
    }
    
    // Config Setup
    m_groundY = height - 32.0f; // Adjusted to be 2 blocks high (16*2=32) or just 1? 
    // User asked to change the ground. Original was 100px high.
    // Let's make it 32px high (2 tiles deep) or just cover the visual area?
    // Let's stick to the visual representation of the ground.
    // Let's make the ground 2 tiles high (32px) so it looks solid.
    
    int numTilesX = static_cast<int>(width / TILE_SIZE) + 1;
    int numTilesY = 2; // Depth of ground

    m_groundVertices.setPrimitiveType(sf::PrimitiveType::Triangles); // SFML 3: Quads removed
    m_groundVertices.resize(numTilesX * numTilesY * 6); // 6 vertices per tile (2 triangles)

    // Texture Coordinates for the specific block
    // User attached a brown block. Usually in NES Mario:
    // Row 0, Col 0: Question Block?
    // Row 0, Col 1: Brick?
    // Row 1, Col 0: Ground?
    // I will try to select the "Ground" block. 
    // In many sheets, Ground is at (0, 0) or (16, 0).
    // Let's try (0, 0) first. If it's a "Question Block", we'll know.
    // Or based on the attached image (brown block with corner details), it looks like the standard breakable brick or the floor block.
    // Let's guess (0, 0) for now.
    // Actually, looking at common spritesheets:
    // (0,0) is often a brick or question block.
    // The ground block is often distinct.
    // I will start with (0,0) and adjust.
    // Width 16, Height 16.
    
    int texU = 0;
    int texV = 0;

    for (int i = 0; i < numTilesX; ++i) {
        for (int j = 0; j < numTilesY; ++j) {
            sf::Vertex* tri = &m_groundVertices[(i * numTilesY + j) * 6];

            float x = i * TILE_SIZE;
            float y = m_groundY + j * TILE_SIZE;

            // Quad positions
            sf::Vector2f p0(x, y);
            sf::Vector2f p1(x + TILE_SIZE, y);
            sf::Vector2f p2(x + TILE_SIZE, y + TILE_SIZE);
            sf::Vector2f p3(x, y + TILE_SIZE);

            // Texture Coords
            sf::Vector2f t0((float)texU, (float)texV);
            sf::Vector2f t1((float)texU + TILE_SIZE, (float)texV);
            sf::Vector2f t2((float)texU + TILE_SIZE, (float)texV + TILE_SIZE);
            sf::Vector2f t3((float)texU, (float)texV + TILE_SIZE);

            // Triangle 1 (0, 1, 2)
            tri[0].position = p0; tri[0].texCoords = t0;
            tri[1].position = p1; tri[1].texCoords = t1;
            tri[2].position = p2; tri[2].texCoords = t2;

            // Triangle 2 (2, 3, 0)
            tri[3].position = p2; tri[3].texCoords = t2;
            tri[4].position = p3; tri[4].texCoords = t3;
            tri[5].position = p0; tri[5].texCoords = t0;
        }
    }

    // Cuerpo estático Box2D v3
    // Physics body needs to match the new visual ground
    // Visual Y starts at m_groundY. Height is numTilesY * TILE_SIZE = 32.
    // Physics Center Y = m_groundY + 16 (half height).
    
    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = (b2Vec2){(width / 2.0f) / Physics::SCALE, (m_groundY + 16.0f) / Physics::SCALE};
    groundDef.type = b2_staticBody; 

    m_groundBodyId = b2CreateBody(m_physics.worldId(), &groundDef);

    // Forma
    float halfWidth = (width / 2.0f) / Physics::SCALE;
    float halfHeight = (32.0f / 2.0f) / Physics::SCALE; // 32px height
    b2Polygon groundBox = b2MakeBox(halfWidth, halfHeight);

    // Fixture
    b2ShapeDef fixtureDef = b2DefaultShapeDef();
    // fixtureDef.friction = 0.6f;
    
    b2CreatePolygonShape(m_groundBodyId, &fixtureDef, &groundBox);

    // Muro Izquierdo (Invisible)
    // Bloquea el movimiento hacia la izquierda de x=0
    b2BodyDef wallDef = b2DefaultBodyDef();
    // Posición: Centrado en x = -10 (ancho 20). Su borde derecho estará en 0.
    wallDef.position = (b2Vec2){-10.0f / Physics::SCALE, (height / 2.0f) / Physics::SCALE};
    wallDef.type = b2_staticBody;
    
    b2BodyId wallId = b2CreateBody(m_physics.worldId(), &wallDef);
    
    b2Polygon wallBox = b2MakeBox(10.0f / Physics::SCALE, (height / 2.0f) / Physics::SCALE); // Altura completa
    b2ShapeDef wallShapeDef = b2DefaultShapeDef();
    // wallShapeDef.friction = 0.0f; // Sin fricción para que no se pegue
    
    b2CreatePolygonShape(wallId, &wallShapeDef, &wallBox);

    // Add a Test Block
    // Position: x=200, y=groundY - 64 (High enough to jump and hit)
    // Note: y grows downwards. groundY is `height - 32`.
    // Let's put it at `height - 32 - 48` = 80px above ground?
    // Player jumps quite high.
    m_blocks.emplace_back(m_physics, 250.0f, m_groundY - 64.0f);
    m_blocks.emplace_back(m_physics, 500.0f, m_groundY - 64.0f);  // Segundo bloque para Fire Flower

    // Spawn Enemies - position at ground level (m_groundY is top of ground)
    m_enemies.push_back(std::make_unique<Goomba>(m_physics, 400.0f, m_groundY));
    m_enemies.push_back(std::make_unique<Goomba>(m_physics, 550.0f, m_groundY));
    m_enemies.push_back(std::make_unique<Koopa>(m_physics, 700.0f, m_groundY));
}

void Level::update(float dt) {
    // Update stomp cooldown
    if (m_stompCooldown > 0.0f) {
        m_stompCooldown -= dt;
    }
    
    for (auto& block : m_blocks) {
        block.update(dt);
    }
    for (auto& item : m_items) {
        item->update(dt);
    }
    
    // Update Enemies
    for (auto& enemy : m_enemies) {
        enemy->update(dt);
    }
    
    // Remove dead Enemies
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { return !e->isAlive(); }),
        m_enemies.end());
}

void Level::checkCollisions(Player& player) {
    // Check Head collision with Blocks
    // Simple AABB check: Player Head (top of sprite) vs Block Bottom
    sf::Vector2f pPos = player.getPosition();
    sf::FloatRect pBounds = player.getBounds();
    
    // Define a small sensor box above the player's head
    // SFML 3: pBounds.top -> pBounds.position.y
    sf::FloatRect headRect({pPos.x - 5, pBounds.position.y - 5}, {10, 10});

    for (size_t i = 0; i < m_blocks.size(); ++i) {
        auto& block = m_blocks[i];
        if (block.isActive()) {
            sf::FloatRect bBounds = block.getBounds();
            // Check intersection (SFML 3: findIntersection returns optional)
            if (headRect.findIntersection(bBounds)) {
                // Trigger hit only if block is Question
                block.hit();
                
                // Spawn Item based on block index and Mario's state
                // Block 0 = Mushroom only
                // Block 1+ = Fire Flower block (Mushroom if small, Fire Flower if big)
                if (i == 0) {
                    // First block always spawns Mushroom
                    m_items.push_back(std::make_unique<Item>(m_physics, block.getPosition().x, block.getPosition().y));
                } else {
                    // Power-up blocks: classic behavior
                    if (player.isBig()) {
                        // Big Mario gets Fire Flower
                        m_items.push_back(std::make_unique<FireFlower>(m_physics, block.getPosition().x, block.getPosition().y));
                    } else {
                        // Small Mario gets Mushroom
                        m_items.push_back(std::make_unique<Item>(m_physics, block.getPosition().x, block.getPosition().y));
                    }
                }
            }
        }
    }

    // Check Item Collisions (Collection)
    for (auto& item : m_items) {
        if (!item->isCollected() && !item->isSpawning()) {
             // Simple Box collision between Player and Item
             if (player.getBounds().findIntersection(item->getBounds())) {
                 item->collect();
                 
                 // Check if it's a Fire Flower
                 FireFlower* fireFlower = dynamic_cast<FireFlower*>(item.get());
                 if (fireFlower) {
                     player.becomeFireMario();
                 } else {
                     player.grow();
                 }
             }
        }
    }

    // Check Enemy Collisions (skip if in stomp cooldown)
    if (m_stompCooldown <= 0.0f) {
        for (auto& enemy : m_enemies) {
            if (!enemy->isAlive()) {
                continue;
            }
            
            sf::FloatRect enemyBounds = enemy->getBounds();
            if (pBounds.findIntersection(enemyBounds)) {
                sf::Vector2f enemyPos = enemy->getPosition();
                // SFML 3: top -> position.y, height -> size.y
                float playerBottom = pBounds.position.y + pBounds.size.y;
                float enemyTop = enemyBounds.position.y;
                
                // Stomp condition (Improved):
                // 1. Player must be falling (velocity y > 0)
                // 2. Player feet must be above the enemy's vertical center (more generous than top edge)
                // 3. Player must have horizontal overlap
                
                b2Vec2 pVel = player.getVelocity();
                bool isFalling = pVel.y > 0.0f;
                
                float enemyCenterY = enemyBounds.position.y + (enemyBounds.size.y / 2.0f);
                bool feetAboveCenter = playerBottom < enemyCenterY + 5.0f; // Allow slightly below center if falling fast
                
                float enemyCenterX = enemyPos.x;
                float playerCenterX = pPos.x;
                float horizontalDistance = std::abs(playerCenterX - enemyCenterX);
                float maxHorizontalDistance = (enemyBounds.size.x / 2.0f) + 6.0f;  // Generous horizontal overlap
                
                bool horizontallyAligned = horizontalDistance < maxHorizontalDistance;
                
                // If falling and reasonably above, count as stomp even if overlapping slightly
                if (isFalling && feetAboveCenter && horizontallyAligned) {
                    // Stomp!
                    enemy->stomp();
                    player.bounce();  // Mario bounces after stomping
                    m_stompCooldown = STOMP_COOLDOWN_TIME;  // Start cooldown
                    std::cout << "Enemy stomped!" << std::endl;
                    break;  // Only process one stomp per frame
                } else {
                    // Check if this is a Koopa shell that can be kicked
                    Koopa* koopa = dynamic_cast<Koopa*>(enemy.get());
                    if (koopa && koopa->isIdleShell()) {
                        // Kick direction based on player position relative to shell
                        float kickDirection = (pPos.x < enemyPos.x) ? 1.0f : -1.0f;
                        
                        // Pass player horizontal speed for dynamic kick
                        float playerSpeed = std::abs(pVel.x);
                        koopa->kick(kickDirection, playerSpeed);
                        
                        m_stompCooldown = STOMP_COOLDOWN_TIME;  // Start cooldown
                        std::cout << "Shell kicked!" << std::endl;
                        break;
                    } else {
                        // Mario takes damage (for now just log)
                        player.takeDamage();
                    }
                }
            }
        }
    }
}

void Level::draw(sf::RenderWindow& window)
{
    window.draw(m_groundVertices, &m_texture);
    
    for (auto& block : m_blocks) {
        block.draw(window);
    }
    for (auto& item : m_items) {
        item->draw(window);
    }
    for (auto& enemy : m_enemies) {
        enemy->draw(window);
    }
}

float Level::groundY() const
{
    // Ensure this matches the visual top of the blocks
    return m_groundY;
}