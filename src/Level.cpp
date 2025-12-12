#include "Level.hpp"
#include "Player.hpp"
#include <algorithm>
#include <iostream>

Level::Level(Physics &physics, float width, float height)
    : m_physics(physics), m_width(width), m_height(height),
      m_stompCooldown(0.0f) {
  // Load Texture
  if (!m_texture.loadFromFile("assets/images/blocks.png")) {
    std::cerr << "Error loading blocks.png" << std::endl;
  }

  // Config Setup
  m_groundY = height - 32.0f;

  // Usar LEVEL_WIDTH para el suelo (nivel extendido)
  int numTilesX = static_cast<int>(LEVEL_WIDTH / TILE_SIZE) + 1;
  int numTilesY = 2; // Depth of ground

  m_groundVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices.resize(numTilesX * numTilesY * 6);

  int texU = 0;
  int texV = 0;

  for (int i = 0; i < numTilesX; ++i) {
    for (int j = 0; j < numTilesY; ++j) {
      sf::Vertex *tri = &m_groundVertices[(i * numTilesY + j) * 6];

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
      tri[0].position = p0;
      tri[0].texCoords = t0;
      tri[1].position = p1;
      tri[1].texCoords = t1;
      tri[2].position = p2;
      tri[2].texCoords = t2;

      // Triangle 2 (2, 3, 0)
      tri[3].position = p2;
      tri[3].texCoords = t2;
      tri[4].position = p3;
      tri[4].texCoords = t3;
      tri[5].position = p0;
      tri[5].texCoords = t0;
    }
  }

  // Cuerpo estático Box2D v3
  b2BodyDef groundDef = b2DefaultBodyDef();
  // Usar LEVEL_WIDTH para el cuerpo físico del suelo
  groundDef.position = (b2Vec2){(LEVEL_WIDTH / 2.0f) / Physics::SCALE,
                                (m_groundY + 16.0f) / Physics::SCALE};
  groundDef.type = b2_staticBody;

  m_groundBodyId = b2CreateBody(m_physics.worldId(), &groundDef);

  // Forma - usar LEVEL_WIDTH para cubrir todo el nivel
  float halfWidth = (LEVEL_WIDTH / 2.0f) / Physics::SCALE;
  float halfHeight = (32.0f / 2.0f) / Physics::SCALE;
  b2Polygon groundBox = b2MakeBox(halfWidth, halfHeight);

  // Fixture
  b2ShapeDef fixtureDef = b2DefaultShapeDef();
  b2CreatePolygonShape(m_groundBodyId, &fixtureDef, &groundBox);

  // Muro Izquierdo (Invisible)
  b2BodyDef wallDef = b2DefaultBodyDef();
  wallDef.position =
      (b2Vec2){-10.0f / Physics::SCALE, (height / 2.0f) / Physics::SCALE};
  wallDef.type = b2_staticBody;

  b2BodyId wallId = b2CreateBody(m_physics.worldId(), &wallDef);

  b2Polygon wallBox =
      b2MakeBox(10.0f / Physics::SCALE, (height / 2.0f) / Physics::SCALE);
  b2ShapeDef wallShapeDef = b2DefaultShapeDef();
  b2CreatePolygonShape(wallId, &wallShapeDef, &wallBox);

  // Bloques distribuidos a lo largo del nivel extendido
  m_blocks.emplace_back(m_physics, 250.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 600.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 900.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 1200.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 1500.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 1800.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 2100.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 2400.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 2700.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 3000.0f, m_groundY - 64.0f);

  // Enemigos distribuidos a lo largo del nivel
  m_enemies.push_back(std::make_unique<Goomba>(m_physics, 400.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Goomba>(m_physics, 550.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Koopa>(m_physics, 700.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Goomba>(m_physics, 1000.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Goomba>(m_physics, 1300.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Koopa>(m_physics, 1600.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Goomba>(m_physics, 1900.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Goomba>(m_physics, 2200.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Koopa>(m_physics, 2500.0f, m_groundY));
  m_enemies.push_back(std::make_unique<Goomba>(m_physics, 2800.0f, m_groundY));
}

void Level::update(float dt) {
  // Update stomp cooldown
  if (m_stompCooldown > 0.0f) {
    m_stompCooldown -= dt;
  }

  for (auto &block : m_blocks) {
    block.update(dt);
  }
  for (auto &item : m_items) {
    item->update(dt);
  }

  // Update Enemies
  for (auto &enemy : m_enemies) {
    enemy->update(dt);
  }

  // Remove dead Enemies
  m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
                                 [](const std::unique_ptr<Enemy> &e) {
                                   return !e->isAlive();
                                 }),
                  m_enemies.end());
}

void Level::checkCollisions(Player &player) {
  // Check Head collision with Blocks
  sf::Vector2f pPos = player.getPosition();
  sf::FloatRect pBounds = player.getBounds();

  // Define a small sensor box above the player's head
  sf::FloatRect headRect({pPos.x - 5, pBounds.position.y - 5}, {10, 10});

  // Check block collisions with Fire Flower logic
  for (size_t i = 0; i < m_blocks.size(); ++i) {
    auto &block = m_blocks[i];
    if (block.isActive()) {
      sf::FloatRect bBounds = block.getBounds();
      if (headRect.findIntersection(bBounds)) {
        block.hit();

        // Spawn Item based on block index and Mario's state
        // Block 0 = Mushroom only
        // Block 1+ = Fire Flower block (Mushroom if small, Fire Flower if big)
        if (i == 0) {
          // First block always spawns Mushroom
          m_items.push_back(std::make_unique<Item>(
              m_physics, block.getPosition().x, block.getPosition().y));
        } else {
          // Power-up blocks: classic behavior
          if (player.isBig()) {
            // Big Mario gets Fire Flower
            m_items.push_back(std::make_unique<FireFlower>(
                m_physics, block.getPosition().x, block.getPosition().y));
          } else {
            // Small Mario gets Mushroom
            m_items.push_back(std::make_unique<Item>(
                m_physics, block.getPosition().x, block.getPosition().y));
          }
        }
      }
    }
  }

  // Check Item Collisions (Collection)
  for (auto &item : m_items) {
    if (!item->isCollected() && !item->isSpawning()) {
      if (player.getBounds().findIntersection(item->getBounds())) {
        item->collect();

        // Check if it's a Fire Flower
        FireFlower *fireFlower = dynamic_cast<FireFlower *>(item.get());
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
    for (auto &enemy : m_enemies) {
      if (!enemy->isAlive()) {
        continue;
      }

      sf::FloatRect enemyBounds = enemy->getBounds();
      if (pBounds.findIntersection(enemyBounds)) {
        sf::Vector2f enemyPos = enemy->getPosition();
        float playerBottom = pBounds.position.y + pBounds.size.y;

        b2Vec2 pVel = player.getVelocity();
        bool isFalling = pVel.y > 0.0f;

        float enemyCenterY =
            enemyBounds.position.y + (enemyBounds.size.y / 2.0f);
        bool feetAboveCenter = playerBottom < enemyCenterY + 5.0f;

        float enemyCenterX = enemyPos.x;
        float playerCenterX = pPos.x;
        float horizontalDistance = std::abs(playerCenterX - enemyCenterX);
        float maxHorizontalDistance = (enemyBounds.size.x / 2.0f) + 6.0f;

        bool horizontallyAligned = horizontalDistance < maxHorizontalDistance;

        if (isFalling && feetAboveCenter && horizontallyAligned) {
          enemy->stomp();
          player.bounce();
          m_stompCooldown = STOMP_COOLDOWN_TIME;
          std::cout << "Enemy stomped!" << std::endl;
          break;
        } else {
          Koopa *koopa = dynamic_cast<Koopa *>(enemy.get());
          if (koopa && koopa->isIdleShell()) {
            float kickDirection = (pPos.x < enemyPos.x) ? 1.0f : -1.0f;
            float playerSpeed = std::abs(pVel.x);
            koopa->kick(kickDirection, playerSpeed);
            m_stompCooldown = STOMP_COOLDOWN_TIME;
            std::cout << "Shell kicked!" << std::endl;
            break;
          } else {
            player.takeDamage();
          }
        }
      }
    }
  }
}

void Level::draw(sf::RenderWindow &window) {
  window.draw(m_groundVertices, &m_texture);

  for (auto &block : m_blocks) {
    block.draw(window);
  }
  for (auto &item : m_items) {
    item->draw(window);
  }
  for (auto &enemy : m_enemies) {
    enemy->draw(window);
  }
}

float Level::groundY() const { return m_groundY; }