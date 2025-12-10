#include "Level.hpp"
#include "Player.hpp"
#include <iostream>

Level::Level(Physics& physics, float width, float height)
: m_physics(physics), m_width(width), m_height(height)
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

    m_groundVertices.setPrimitiveType(sf::Quads);
    m_groundVertices.resize(numTilesX * numTilesY * 4);

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
            sf::Vertex* quad = &m_groundVertices[(i * numTilesY + j) * 4];

            float x = i * TILE_SIZE;
            float y = m_groundY + j * TILE_SIZE;

            // Position
            quad[0].position = sf::Vector2f(x, y);
            quad[1].position = sf::Vector2f(x + TILE_SIZE, y);
            quad[2].position = sf::Vector2f(x + TILE_SIZE, y + TILE_SIZE);
            quad[3].position = sf::Vector2f(x, y + TILE_SIZE);

            // Texture Coords
            quad[0].texCoords = sf::Vector2f(texU, texV);
            quad[1].texCoords = sf::Vector2f(texU + TILE_SIZE, texV);
            quad[2].texCoords = sf::Vector2f(texU + TILE_SIZE, texV + TILE_SIZE);
            quad[3].texCoords = sf::Vector2f(texU, texV + TILE_SIZE);
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
    fixtureDef.friction = 0.6f;
    
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
    wallShapeDef.friction = 0.0f; // Sin fricción para que no se pegue
    
    b2CreatePolygonShape(wallId, &wallShapeDef, &wallBox);

    // Add a Test Block
    // Position: x=200, y=groundY - 64 (High enough to jump and hit)
    // Note: y grows downwards. groundY is `height - 32`.
    // Let's put it at `height - 32 - 48` = 80px above ground?
    // Player jumps quite high.
    m_blocks.emplace_back(m_physics, 250.0f, m_groundY - 64.0f);
}

void Level::update(float dt) {
    for (auto& block : m_blocks) {
        block.update(dt);
    }
    for (auto& item : m_items) {
        item->update(dt);
    }
}

void Level::checkCollisions(Player& player) {
    // Check Head collision with Blocks
    // Simple AABB check: Player Head (top of sprite) vs Block Bottom
    sf::Vector2f pPos = player.getPosition();
    sf::FloatRect pBounds = player.getBounds();
    
    // Define a small sensor box above the player's head
    sf::FloatRect headRect(pPos.x - 5, pBounds.top - 5, 10, 10);

    for (auto& block : m_blocks) {
        if (block.isActive()) {
            sf::FloatRect bBounds = block.getBounds();
            // Check intersection
            if (headRect.intersects(bBounds)) {
                // Check if player is moving UP (Velocity Y < 0)
                // We need access to player velocity or assume hit if head triggers
                // ideally passed in, but let's assume valid hit for now
                // Actually, trigger hit only if block is Question
                block.hit();
                
                // Spawn Item
                // Only if it WAS a question block (now empty)
                // If we had a mechanism to know if hit succeeded...
                // Let's assume hitting it spawns item for now (simplified)
                // Better: check if items vector is empty to avoid dupes for this demo
                if (m_items.empty()) {
                     m_items.push_back(std::make_unique<Item>(m_physics, block.getPosition().x, block.getPosition().y));
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
}

float Level::groundY() const
{
    // Ensure this matches the visual top of the blocks
    return m_groundY;
}