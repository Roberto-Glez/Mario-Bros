#include "Level.hpp"
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
}

void Level::draw(sf::RenderWindow& window)
{
    window.draw(m_groundVertices, &m_texture);
}

float Level::groundY() const
{
    // Ensure this matches the visual top of the blocks
    return m_groundY;
}