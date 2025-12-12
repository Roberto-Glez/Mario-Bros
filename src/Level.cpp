#include "Level.hpp"
#include "Player.hpp"
#include <algorithm>
#include <iostream>

Level::Level(Physics &physics, float width, float height)
    : m_physics(physics), m_width(width), m_height(height),
      m_stompCooldown(0.0f) {
  // Load Textures
  if (!m_texture.loadFromFile("assets/images/tilesets.png")) {
    std::cerr << "Error loading tilesets.png" << std::endl;
  }
  if (!m_texture2.loadFromFile("assets/images/plataformas.png")) {
    std::cerr << "Error loading plataformas.png" << std::endl;
  }

  // Config Setup
  m_groundY = height - 32.0f;

  // Usar tiles de 32x32 en pantalla (sprite 16x16 escalado a 2x)
  static constexpr int DISPLAY_TILE_SIZE =
      32;                                  // Tamaño en pantalla (escalado 2x)
  static constexpr int TEX_TILE_SIZE = 16; // Tamaño del sprite en textura
  int numTilesX = static_cast<int>(LEVEL_WIDTH / DISPLAY_TILE_SIZE) + 1;

  // Sección alternativa: desde X=640 por 17 bloques
  static constexpr int ALT_START_TILE = 640 / DISPLAY_TILE_SIZE; // Tile 20
  static constexpr int ALT_NUM_TILES = 17;
  static constexpr int ALT_END_TILE = ALT_START_TILE + ALT_NUM_TILES; // Tile 37

  // Contar tiles normales (excluyendo la sección alternativa)
  int normalTiles = numTilesX - ALT_NUM_TILES;

  m_groundVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices.resize(normalTiles * 6);

  // Segundo VertexArray para la sección alternativa
  m_groundVertices2.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices2.resize(ALT_NUM_TILES * 6);

  int texU = 80; // Sprite normal de tilesets.png
  int texV = 176;

  int texU2 = 272; // Sprite alternativo de plataformas.png
  int texV2 = 16;

  int normalIdx = 0;
  int altIdx = 0;

  for (int i = 0; i < numTilesX; ++i) {
    float x = i * DISPLAY_TILE_SIZE;
    float y = m_groundY;

    // Quad positions (32x32 en pantalla)
    sf::Vector2f p0(x, y);
    sf::Vector2f p1(x + DISPLAY_TILE_SIZE, y);
    sf::Vector2f p2(x + DISPLAY_TILE_SIZE, y + DISPLAY_TILE_SIZE);
    sf::Vector2f p3(x, y + DISPLAY_TILE_SIZE);

    // Determinar si este tile está en la sección alternativa
    bool isAltSection = (i >= ALT_START_TILE && i < ALT_END_TILE);

    if (isAltSection) {
      // Usar textura alternativa (plataformas.png)
      sf::Vertex *tri = &m_groundVertices2[altIdx * 6];

      sf::Vector2f t0((float)texU2, (float)texV2);
      sf::Vector2f t1((float)texU2 + TEX_TILE_SIZE, (float)texV2);
      sf::Vector2f t2((float)texU2 + TEX_TILE_SIZE,
                      (float)texV2 + TEX_TILE_SIZE);
      sf::Vector2f t3((float)texU2, (float)texV2 + TEX_TILE_SIZE);

      tri[0].position = p0;
      tri[0].texCoords = t0;
      tri[1].position = p1;
      tri[1].texCoords = t1;
      tri[2].position = p2;
      tri[2].texCoords = t2;
      tri[3].position = p2;
      tri[3].texCoords = t2;
      tri[4].position = p3;
      tri[4].texCoords = t3;
      tri[5].position = p0;
      tri[5].texCoords = t0;

      altIdx++;
    } else {
      // Usar textura normal (tilesets.png)
      sf::Vertex *tri = &m_groundVertices[normalIdx * 6];

      sf::Vector2f t0((float)texU, (float)texV);
      sf::Vector2f t1((float)texU + TEX_TILE_SIZE, (float)texV);
      sf::Vector2f t2((float)texU + TEX_TILE_SIZE, (float)texV + TEX_TILE_SIZE);
      sf::Vector2f t3((float)texU, (float)texV + TEX_TILE_SIZE);

      tri[0].position = p0;
      tri[0].texCoords = t0;
      tri[1].position = p1;
      tri[1].texCoords = t1;
      tri[2].position = p2;
      tri[2].texCoords = t2;
      tri[3].position = p2;
      tri[3].texCoords = t2;
      tri[4].position = p3;
      tri[4].texCoords = t3;
      tri[5].position = p0;
      tri[5].texCoords = t0;

      normalIdx++;
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
  m_blocks.emplace_back(m_physics, 1800.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 2100.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 2400.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 2700.0f, m_groundY - 64.0f);
  m_blocks.emplace_back(m_physics, 3000.0f, m_groundY - 64.0f);

  // Enemigos distribuidos a lo largo del nivel (DESACTIVADOS TEMPORALMENTE)
  // m_enemies.push_back(std::make_unique<Goomba>(m_physics, 400.0f,
  // m_groundY)); m_enemies.push_back(std::make_unique<Goomba>(m_physics,
  // 550.0f, m_groundY)); m_enemies.push_back(std::make_unique<Koopa>(m_physics,
  // 700.0f, m_groundY));
  // m_enemies.push_back(std::make_unique<Goomba>(m_physics, 1000.0f,
  // m_groundY)); m_enemies.push_back(std::make_unique<Goomba>(m_physics,
  // 1300.0f, m_groundY));
  // m_enemies.push_back(std::make_unique<Koopa>(m_physics, 1600.0f,
  // m_groundY)); m_enemies.push_back(std::make_unique<Goomba>(m_physics,
  // 1900.0f, m_groundY));
  // m_enemies.push_back(std::make_unique<Goomba>(m_physics, 2200.0f,
  // m_groundY)); m_enemies.push_back(std::make_unique<Koopa>(m_physics,
  // 2500.0f, m_groundY));
  // m_enemies.push_back(std::make_unique<Goomba>(m_physics, 2800.0f,
  // m_groundY));
  // Plataforma sólida naranja en X=320, 96px arriba del suelo (3 bloques de
  // ancho) Usamos UN SOLO cuerpo físico para evitar ghost collisions
  {
    Platform plat;
    plat.x = 320.0f;
    plat.y = m_groundY - 96.0f; // 96px arriba del suelo
    plat.width = 96.0f;         // 3 bloques de 32px = 96px total
    plat.height = 32.0f;

    // Cuerpo físico estático (uno solo para toda la plataforma)
    b2BodyDef platBodyDef = b2DefaultBodyDef();
    platBodyDef.type = b2_staticBody;
    platBodyDef.position =
        (b2Vec2){(plat.x + plat.width / 2.0f) / Physics::SCALE,
                 (plat.y + plat.height / 2.0f) / Physics::SCALE};
    plat.bodyId = b2CreateBody(m_physics.worldId(), &platBodyDef);

    b2Polygon platBox = b2MakeBox((plat.width / 2.0f) / Physics::SCALE,
                                  (plat.height / 2.0f) / Physics::SCALE);
    b2ShapeDef platShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(plat.bodyId, &platShapeDef, &platBox);

    // Visual naranja (dibuja los 3 bloques como uno solo)
    plat.shape.setSize({plat.width, plat.height});
    plat.shape.setPosition({plat.x, plat.y});
    plat.shape.setFillColor(sf::Color(255, 165, 0)); // Naranja

    m_platforms.push_back(plat);
  }

  // Cargar textura de plantas para decoraciones
  if (!m_plantasTexture.loadFromFile("assets/images/plantas.png")) {
    std::cerr << "Error loading plantas.png" << std::endl;
  }

  // Sprite decorativo en X=256 (planta de fondo)
  {
    sf::Sprite decoration(m_plantasTexture);
    decoration.setTextureRect(
        sf::IntRect({5, 150}, {16, 16})); // Sprite en (5,150) de 16x16
    decoration.setScale({2.0f, 2.0f});    // Escalar a 32x32
    decoration.setPosition(
        {256.0f, m_groundY - 32.0f}); // Posicionado justo encima del suelo
    m_decorations.push_back(decoration);
  }

  // Sprite decorativo azul en X=288 (al lado del anterior)
  {
    sf::Sprite decoration(m_plantasTexture);
    decoration.setTextureRect(sf::IntRect({5, 150}, {16, 16})); // Mismo sprite
    decoration.setScale({2.0f, 2.0f});                   // Escalar a 32x32
    decoration.setPosition({288.0f, m_groundY - 32.0f}); // Al lado derecho
    decoration.setColor(sf::Color(100, 150, 255));       // Tinte azul
    m_decorations.push_back(decoration);
  }
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

  // Update Fireballs
  for (auto &fireball : m_fireballs) {
    fireball->update(dt);
  }

  // Check Fireball vs Enemy collisions
  for (auto &fireball : m_fireballs) {
    if (!fireball->isAlive())
      continue;

    for (auto &enemy : m_enemies) {
      if (!enemy->isAlive())
        continue;

      if (fireball->getBounds().findIntersection(enemy->getBounds())) {
        // Check if it's a Koopa shell
        Koopa *koopa = dynamic_cast<Koopa *>(enemy.get());
        if (koopa && koopa->isShell()) {
          // Kill shell with special animation
          koopa->killByFireball();
          std::cout << "Fireball killed Koopa shell!" << std::endl;
        } else {
          // Regular enemy - use stomp
          enemy->stomp();
          std::cout << "Fireball hit enemy!" << std::endl;
        }
        fireball->destroy();
        break;
      }
    }
  }

  // Remove dead Enemies
  m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
                                 [](const std::unique_ptr<Enemy> &e) {
                                   return !e->isAlive();
                                 }),
                  m_enemies.end());

  // Remove dead Fireballs
  m_fireballs.erase(std::remove_if(m_fireballs.begin(), m_fireballs.end(),
                                   [](const std::unique_ptr<Fireball> &f) {
                                     return !f->isAlive();
                                   }),
                    m_fireballs.end());
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
        // Trigger hit - only spawn item if first hit
        if (block.hit()) {
          // Spawn Item based on block index and Mario's state
          // Block 0 = Mushroom only
          // Block 1+ = Fire Flower block (Mushroom if small, Fire Flower if
          // big)
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

      // Skip dying Koopa shells (hit by fireball)
      Koopa *dyingKoopa = dynamic_cast<Koopa *>(enemy.get());
      if (dyingKoopa && dyingKoopa->isStomped()) {
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
  // Dibujar fondo de tablero de ajedrez (cuadros de 32x32)
  // Alineado desde el suelo hacia arriba
  static constexpr float CHECKER_SIZE = 32.0f;
  int numCols = static_cast<int>(LEVEL_WIDTH / CHECKER_SIZE) + 1;

  // Calcular cuántas filas desde el suelo hacia arriba
  float bottomY = m_groundY - CHECKER_SIZE; // Última fila termina en el suelo
  int numRows = static_cast<int>(m_groundY / CHECKER_SIZE) + 1;

  sf::RectangleShape checker({CHECKER_SIZE, CHECKER_SIZE});

  for (int row = 0; row < numRows; ++row) {
    // Calcular Y desde el suelo hacia arriba
    float y = bottomY - (row * CHECKER_SIZE);
    if (y < -CHECKER_SIZE)
      break; // No dibujar fuera de pantalla

    for (int col = 0; col < numCols; ++col) {
      // Alternar colores como tablero de ajedrez
      if ((row + col) % 2 == 0) {
        checker.setFillColor(sf::Color::White);
      } else {
        checker.setFillColor(sf::Color::Black);
      }
      checker.setPosition({col * CHECKER_SIZE, y});
      window.draw(checker);
    }
  }

  // Dibujar suelo (sección normal con tilesets.png)
  window.draw(m_groundVertices, &m_texture);
  // Dibujar suelo (sección alternativa con plataformas.png)
  window.draw(m_groundVertices2, &m_texture2);

  // Dibujar decoraciones de fondo
  for (auto &decoration : m_decorations) {
    window.draw(decoration);
  }

  for (auto &block : m_blocks) {
    block.draw(window);
  }
  for (auto &item : m_items) {
    item->draw(window);
  }
  for (auto &enemy : m_enemies) {
    enemy->draw(window);
  }
  for (auto &fireball : m_fireballs) {
    fireball->draw(window);
  }

  // Dibujar plataformas sólidas
  for (auto &plat : m_platforms) {
    window.draw(plat.shape);
  }

  // Marcadores de debug para identificar límites de pantalla
  // Cada pantalla es de 800px de ancho, hay 4 pantallas
  static constexpr float SCREEN_WIDTH = 800.0f;
  static constexpr float SCREEN_HEIGHT = 600.0f;
  static constexpr int NUM_SCREENS = 4;
  static constexpr float MARKER_SIZE = 32.0f;

  sf::RectangleShape marker({MARKER_SIZE, MARKER_SIZE});

  // Colores diferentes para cada pantalla
  sf::Color screenColors[NUM_SCREENS] = {
      sf::Color::Red,   // Pantalla 1
      sf::Color::Green, // Pantalla 2
      sf::Color::Blue,  // Pantalla 3
      sf::Color::Yellow // Pantalla 4
  };

  for (int screen = 0; screen < NUM_SCREENS; ++screen) {
    float screenStartX = screen * SCREEN_WIDTH;
    float screenEndX = screenStartX + SCREEN_WIDTH - MARKER_SIZE;

    marker.setFillColor(screenColors[screen]);

    // Esquina superior izquierda
    marker.setPosition({screenStartX, 0.0f});
    window.draw(marker);

    // Esquina superior derecha
    marker.setPosition({screenEndX, 0.0f});
    window.draw(marker);

    // Esquina inferior izquierda (encima del suelo)
    marker.setPosition({screenStartX, m_groundY - MARKER_SIZE});
    window.draw(marker);

    // Esquina inferior derecha (encima del suelo)
    marker.setPosition({screenEndX, m_groundY - MARKER_SIZE});
    window.draw(marker);
  }
}

void Level::spawnFireball(float x, float y, float direction) {
  m_fireballs.push_back(std::make_unique<Fireball>(m_physics, x, y, direction));
}

float Level::groundY() const { return m_groundY; }