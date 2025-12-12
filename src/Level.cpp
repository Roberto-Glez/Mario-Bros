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
  static constexpr int ALT_NUM_TILES =
      16; // Reducido de 17 a 16 para excluir tile 37
  static constexpr int ALT_END_TILE = ALT_START_TILE + ALT_NUM_TILES; // Tile 37

  // Tercera sección: desde X=1184 (tile 37) hasta el final
  static constexpr int THIRD_START_TILE = 1184 / DISPLAY_TILE_SIZE; // Tile 37
  int thirdNumTiles = numTilesX - THIRD_START_TILE;

  // Contar tiles normales (excluyendo las secciones alternativa y tercera)
  int normalTiles = numTilesX - ALT_NUM_TILES - thirdNumTiles;

  m_groundVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices.resize(normalTiles * 6);

  // Segundo VertexArray para la sección alternativa
  m_groundVertices2.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices2.resize(ALT_NUM_TILES * 6);

  // Tercer VertexArray para la tercera sección
  m_groundVertices3.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices3.resize(thirdNumTiles * 6);

  int texU = 80; // Sprite normal de tilesets.png
  int texV = 176;

  int texU2 = 272; // Sprite alternativo de plataformas.png
  int texV2 = 16;

  int texU3 = 240; // Sprite tercera sección de tilesets.png
  int texV3 = 96;

  int normalIdx = 0;
  int altIdx = 0;
  int thirdIdx = 0;

  for (int i = 0; i < numTilesX; ++i) {
    float x = i * DISPLAY_TILE_SIZE;
    float y = m_groundY;

    // Quad positions (32x32 en pantalla)
    sf::Vector2f p0(x, y);
    sf::Vector2f p1(x + DISPLAY_TILE_SIZE, y);
    sf::Vector2f p2(x + DISPLAY_TILE_SIZE, y + DISPLAY_TILE_SIZE);
    sf::Vector2f p3(x, y + DISPLAY_TILE_SIZE);

    // Determinar qué sección usar
    bool isAltSection = (i >= ALT_START_TILE && i < ALT_END_TILE);
    bool isThirdSection = (i >= THIRD_START_TILE);

    if (isThirdSection) {
      // Usar tercera textura (tilesets.png sprite 240,96)
      sf::Vertex *tri = &m_groundVertices3[thirdIdx * 6];

      sf::Vector2f t0((float)texU3, (float)texV3);
      sf::Vector2f t1((float)texU3 + TEX_TILE_SIZE, (float)texV3);
      sf::Vector2f t2((float)texU3 + TEX_TILE_SIZE,
                      (float)texV3 + TEX_TILE_SIZE);
      sf::Vector2f t3((float)texU3, (float)texV3 + TEX_TILE_SIZE);

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

      thirdIdx++;
    } else if (isAltSection) {
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
  // Plataforma con textura en X=320, 96px arriba del suelo (3 bloques de ancho)
  {
    Platform plat;
    plat.x = 320.0f;
    plat.y = m_groundY - 96.0f;
    plat.width = 96.0f;  // 3 bloques
    plat.height = 32.0f; // 1 bloque

    // Cuerpo físico estático
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

    // Visual con tiles de tilesets.png (240,96) 16x16 escalado a 32x32
    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        // Posiciones del quad (32x32)
        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        // Coordenadas de textura (16x16 en la imagen)
        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Plataforma con textura en X=1280, justo encima del suelo
  {
    Platform plat;
    plat.x = 1280.0f;
    plat.y = m_groundY - 32.0f;
    plat.width = 96.0f;
    plat.height = 32.0f;

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Plataforma con textura en X=1856 (bloque 58), 2 bloques de altura
  {
    Platform plat;
    plat.x = 1856.0f;
    plat.y = m_groundY - 64.0f;
    plat.width = 192.0f;
    plat.height = 64.0f;

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Plataforma con textura en X=1952 (bloque 61), 5 bloques de altura
  {
    Platform plat;
    plat.x = 1952.0f;
    plat.y = m_groundY - 160.0f;
    plat.width = 128.0f;
    plat.height = 160.0f;

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Plataforma con textura en X=2080 (bloque 65), 8 bloques de altura
  {
    Platform plat;
    plat.x = 2080.0f;            // Bloque horizontal 65 (65 * 32 = 2080)
    plat.y = m_groundY - 256.0f; // 8 bloques arriba del suelo (8 * 32 = 256px)
    plat.width = 192.0f;         // 6 bloques de ancho (6 * 32 = 192px)
    plat.height = 256.0f;        // 8 bloques de altura (8 * 32 = 256px)

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Bloque naranja en X=2368, a 256px del suelo (1 bloque)
  {
    Platform plat;
    plat.x = 2368.0f;            // Pixel 2368 horizontal
    plat.y = m_groundY - 256.0f; // 256px arriba del suelo
    plat.width = 32.0f;          // 1 bloque de ancho
    plat.height = 32.0f;         // 1 bloque de altura

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

    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(6);

    sf::Vector2f p0(plat.x, plat.y);
    sf::Vector2f p1(plat.x + 32.0f, plat.y);
    sf::Vector2f p2(plat.x + 32.0f, plat.y + 32.0f);
    sf::Vector2f p3(plat.x, plat.y + 32.0f);

    sf::Vector2f t0(240.0f, 96.0f);
    sf::Vector2f t1(256.0f, 96.0f);
    sf::Vector2f t2(256.0f, 112.0f);
    sf::Vector2f t3(240.0f, 112.0f);

    plat.vertices[0].position = p0;
    plat.vertices[0].texCoords = t0;
    plat.vertices[1].position = p1;
    plat.vertices[1].texCoords = t1;
    plat.vertices[2].position = p2;
    plat.vertices[2].texCoords = t2;
    plat.vertices[3].position = p2;
    plat.vertices[3].texCoords = t2;
    plat.vertices[4].position = p3;
    plat.vertices[4].texCoords = t3;
    plat.vertices[5].position = p0;
    plat.vertices[5].texCoords = t0;

    m_platforms.push_back(plat);
  }

  // Bloque naranja en X=2496, a 320px del suelo (1 bloque)
  {
    Platform plat;
    plat.x = 2496.0f;            // Pixel 2496 horizontal
    plat.y = m_groundY - 320.0f; // 320px arriba del suelo
    plat.width = 32.0f;          // 1 bloque de ancho
    plat.height = 32.0f;         // 1 bloque de altura

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

    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(6);

    sf::Vector2f p0(plat.x, plat.y);
    sf::Vector2f p1(plat.x + 32.0f, plat.y);
    sf::Vector2f p2(plat.x + 32.0f, plat.y + 32.0f);
    sf::Vector2f p3(plat.x, plat.y + 32.0f);

    sf::Vector2f t0(240.0f, 96.0f);
    sf::Vector2f t1(256.0f, 96.0f);
    sf::Vector2f t2(256.0f, 112.0f);
    sf::Vector2f t3(240.0f, 112.0f);

    plat.vertices[0].position = p0;
    plat.vertices[0].texCoords = t0;
    plat.vertices[1].position = p1;
    plat.vertices[1].texCoords = t1;
    plat.vertices[2].position = p2;
    plat.vertices[2].texCoords = t2;
    plat.vertices[3].position = p2;
    plat.vertices[3].texCoords = t2;
    plat.vertices[4].position = p3;
    plat.vertices[4].texCoords = t3;
    plat.vertices[5].position = p0;
    plat.vertices[5].texCoords = t0;

    m_platforms.push_back(plat);
  }

  // Plataforma con textura en X=2688, 5 bloques de alto × 7 de ancho
  {
    Platform plat;
    plat.x = 2688.0f;            // Pixel 2688 horizontal
    plat.y = m_groundY - 160.0f; // 5 bloques arriba del suelo (5 * 32 = 160px)
    plat.width = 224.0f;         // 7 bloques de ancho (7 * 32 = 224px)
    plat.height = 160.0f;        // 5 bloques de altura (5 * 32 = 160px)

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Isla flotante en X=3040, separada 160px del suelo (10 bloques × 3 bloques)
  {
    Platform plat;
    plat.x = 3040.0f;            // Pixel 3040 horizontal
    plat.y = m_groundY - 256.0f; // 160px de separación + 96px de altura = 256px
    plat.width = 320.0f;         // 10 bloques de ancho (10 * 32 = 320px)
    plat.height = 96.0f;         // 3 bloques de altura (3 * 32 = 96px)

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Plataforma en X=3520, 11 bloques de altura × 8 bloques de ancho
  {
    Platform plat;
    plat.x = 3520.0f;            // Pixel 3520 horizontal
    plat.y = m_groundY - 352.0f; // 11 bloques desde el suelo (11 * 32 = 352px)
    plat.width = 256.0f;         // 8 bloques de ancho (8 * 32 = 256px)
    plat.height = 352.0f;        // 11 bloques de altura (11 * 32 = 352px)

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Sección izquierda de la plataforma (antes del pasaje)
  // X=4000, ancho=288px (9 bloques), altura=14 bloques
  {
    Platform plat;
    plat.x = 4000.0f;
    plat.y = m_groundY - 448.0f;
    plat.width = 288.0f;  // 9 bloques de ancho
    plat.height = 448.0f; // 14 bloques de altura

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Sección debajo del pasaje (7 bloques de altura desde suelo)
  // X=4288, ancho=352px (11 bloques), altura=7 bloques (224px)
  {
    Platform plat;
    plat.x = 4288.0f;
    plat.y = m_groundY - 224.0f; // 7 bloques desde el suelo
    plat.width = 352.0f;         // 11 bloques de ancho
    plat.height = 224.0f;        // 7 bloques de altura

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Sección arriba del pasaje (desde 12 bloques de altura hasta el tope)
  // X=4288, ancho=352px (11 bloques), altura=2 bloques (64px)
  {
    Platform plat;
    plat.x = 4288.0f;
    plat.y = m_groundY - 448.0f; // Desde el tope de la plataforma original
    plat.width = 352.0f;         // 11 bloques de ancho
    plat.height = 64.0f;         // 2 bloques de altura (14 - 7 - 5 = 2)

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Bloque flotante en X=3776, a 3 bloques de altura (96px)
  {
    Platform plat;
    plat.x = 3776.0f;
    plat.y = m_groundY - 96.0f;
    plat.width = 32.0f;
    plat.height = 32.0f;

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

    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(6);

    sf::Vector2f p0(plat.x, plat.y);
    sf::Vector2f p1(plat.x + 32.0f, plat.y);
    sf::Vector2f p2(plat.x + 32.0f, plat.y + 32.0f);
    sf::Vector2f p3(plat.x, plat.y + 32.0f);

    sf::Vector2f t0(240.0f, 96.0f);
    sf::Vector2f t1(256.0f, 96.0f);
    sf::Vector2f t2(256.0f, 112.0f);
    sf::Vector2f t3(240.0f, 112.0f);

    plat.vertices[0].position = p0;
    plat.vertices[0].texCoords = t0;
    plat.vertices[1].position = p1;
    plat.vertices[1].texCoords = t1;
    plat.vertices[2].position = p2;
    plat.vertices[2].texCoords = t2;
    plat.vertices[3].position = p2;
    plat.vertices[3].texCoords = t2;
    plat.vertices[4].position = p3;
    plat.vertices[4].texCoords = t3;
    plat.vertices[5].position = p0;
    plat.vertices[5].texCoords = t0;

    m_platforms.push_back(plat);
  }

  // Bloque flotante en X=3904, a 6 bloques de altura (192px)
  {
    Platform plat;
    plat.x = 3904.0f;
    plat.y = m_groundY - 192.0f;
    plat.width = 32.0f;
    plat.height = 32.0f;

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

    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(6);

    sf::Vector2f p0(plat.x, plat.y);
    sf::Vector2f p1(plat.x + 32.0f, plat.y);
    sf::Vector2f p2(plat.x + 32.0f, plat.y + 32.0f);
    sf::Vector2f p3(plat.x, plat.y + 32.0f);

    sf::Vector2f t0(240.0f, 96.0f);
    sf::Vector2f t1(256.0f, 96.0f);
    sf::Vector2f t2(256.0f, 112.0f);
    sf::Vector2f t3(240.0f, 112.0f);

    plat.vertices[0].position = p0;
    plat.vertices[0].texCoords = t0;
    plat.vertices[1].position = p1;
    plat.vertices[1].texCoords = t1;
    plat.vertices[2].position = p2;
    plat.vertices[2].texCoords = t2;
    plat.vertices[3].position = p2;
    plat.vertices[3].texCoords = t2;
    plat.vertices[4].position = p3;
    plat.vertices[4].texCoords = t3;
    plat.vertices[5].position = p0;
    plat.vertices[5].texCoords = t0;

    m_platforms.push_back(plat);
  }

  // Bloque flotante en X=3776, a 9 bloques de altura (288px)
  {
    Platform plat;
    plat.x = 3776.0f;
    plat.y = m_groundY - 288.0f;
    plat.width = 32.0f;
    plat.height = 32.0f;

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

    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(6);

    sf::Vector2f p0(plat.x, plat.y);
    sf::Vector2f p1(plat.x + 32.0f, plat.y);
    sf::Vector2f p2(plat.x + 32.0f, plat.y + 32.0f);
    sf::Vector2f p3(plat.x, plat.y + 32.0f);

    sf::Vector2f t0(240.0f, 96.0f);
    sf::Vector2f t1(256.0f, 96.0f);
    sf::Vector2f t2(256.0f, 112.0f);
    sf::Vector2f t3(240.0f, 112.0f);

    plat.vertices[0].position = p0;
    plat.vertices[0].texCoords = t0;
    plat.vertices[1].position = p1;
    plat.vertices[1].texCoords = t1;
    plat.vertices[2].position = p2;
    plat.vertices[2].texCoords = t2;
    plat.vertices[3].position = p2;
    plat.vertices[3].texCoords = t2;
    plat.vertices[4].position = p3;
    plat.vertices[4].texCoords = t3;
    plat.vertices[5].position = p0;
    plat.vertices[5].texCoords = t0;

    m_platforms.push_back(plat);
  }

  // Isla naranja en X=4640, a 5 bloques de altura, 5 bloques de ancho, 2 de
  // alto
  {
    Platform plat;
    plat.x = 4640.0f; // Pixel 4640 horizontal
    plat.y =
        m_groundY - 224.0f; // 5 bloques separación + 2 bloques altura = 224px
    plat.width = 160.0f;    // 5 bloques de ancho (5 * 32 = 160px)
    plat.height = 64.0f;    // 2 bloques de altura (2 * 32 = 64px)

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        sf::Vector2f t0(240.0f, 96.0f);
        sf::Vector2f t1(256.0f, 96.0f);
        sf::Vector2f t2(256.0f, 112.0f);
        sf::Vector2f t3(240.0f, 112.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

    m_platforms.push_back(plat);
  }

  // Isla en X=4800, separada 5 bloques del suelo, 25 bloques de ancho, 25 de
  // alto
  {
    Platform plat;
    plat.x = 4800.0f; // Pixel 4800 horizontal
    plat.y =
        m_groundY - 960.0f; // 5 bloques separación + 25 bloques altura = 960px
    plat.width = 800.0f;    // 25 bloques de ancho (25 * 32 = 800px)
    plat.height = 800.0f;   // 25 bloques de altura (25 * 32 = 800px)

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

    int tilesX = static_cast<int>(plat.width / 32.0f);
    int tilesY = static_cast<int>(plat.height / 32.0f);
    plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    plat.vertices.resize(tilesX * tilesY * 6);

    for (int ty = 0; ty < tilesY; ++ty) {
      for (int tx = 0; tx < tilesX; ++tx) {
        int idx = (ty * tilesX + tx) * 6;
        float px = plat.x + tx * 32.0f;
        float py = plat.y + ty * 32.0f;

        sf::Vector2f p0(px, py);
        sf::Vector2f p1(px + 32.0f, py);
        sf::Vector2f p2(px + 32.0f, py + 32.0f);
        sf::Vector2f p3(px, py + 32.0f);

        // Sprite de tilesets.png en (320,16) de 16x16
        sf::Vector2f t0(320.0f, 16.0f);
        sf::Vector2f t1(336.0f, 16.0f);
        sf::Vector2f t2(336.0f, 32.0f);
        sf::Vector2f t3(320.0f, 32.0f);

        plat.vertices[idx + 0].position = p0;
        plat.vertices[idx + 0].texCoords = t0;
        plat.vertices[idx + 1].position = p1;
        plat.vertices[idx + 1].texCoords = t1;
        plat.vertices[idx + 2].position = p2;
        plat.vertices[idx + 2].texCoords = t2;
        plat.vertices[idx + 3].position = p2;
        plat.vertices[idx + 3].texCoords = t2;
        plat.vertices[idx + 4].position = p3;
        plat.vertices[idx + 4].texCoords = t3;
        plat.vertices[idx + 5].position = p0;
        plat.vertices[idx + 5].texCoords = t0;
      }
    }

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
      // Skip dying enemies (except Koopa shells which need physical
      // interaction) Check if it's a Koopa to allow shell interactions even if
      // stomped
      Koopa *koopaEnemy = dynamic_cast<Koopa *>(enemy.get());
      bool isKoopaShell = (koopaEnemy && koopaEnemy->isShell());

      // If it's stomped and NOT a shell, skip it (standard behavior)
      if (enemy->isStomped() && !isKoopaShell) {
        continue;
      }

      // If it IS a shell, but it's dying (falling off screen), skip it
      if (isKoopaShell && koopaEnemy->isStomped() && !koopaEnemy->isShell()) {
        continue;
      }

      // Custom Hitbox Logic: "Strict Stomp"
      sf::Vector2f enemyPos = enemy->getPosition();
      
      // Damage Hitbox: Very Wide (24) to ensure any side contact is lethal.
      // Sprite is ~21 wide. 24 extends slightly beyond visual to punish side touches.
      float dmgW = 24.0f; 
      float dmgH = 18.0f; // Body height
      sf::FloatRect damageBox({enemyPos.x - dmgW/2.0f, enemyPos.y - dmgH}, {dmgW, dmgH});
      
      // Stomp Hitbox: Very Narrow (12) and Top Only.
      // You must land ALMOST CENTERED on the head.
      // If you hit the "shoulder" (side-top), you miss this box and hit damageBox instead.
      float stompW = 12.0f; 
      float stompH = 6.0f;
      sf::FloatRect stompBox({enemyPos.x - stompW/2.0f, enemyPos.y - dmgH - stompH}, {stompW, stompH});

      // Players velocity
      b2Vec2 pVel = player.getVelocity();
      // Must be falling significantly (> 1.0f) to count as a stomp
      bool isFalling = pVel.y > 1.0f;

      // Check Stomp Intersection First
      if (isFalling && pBounds.findIntersection(stompBox)) {
          enemy->stomp();
          player.bounce();
          m_stompCooldown = STOMP_COOLDOWN_TIME;
          std::cout << "Enemy stomped (Strict Custom Hitbox)!" << std::endl;
          break;
      }
      // Check Damage Intersection Second
      else if (pBounds.findIntersection(damageBox)) {
          Koopa *koopa = dynamic_cast<Koopa *>(enemy.get());
          if (koopa && koopa->isIdleShell()) {
            float kickDirection = (player.getPosition().x < enemyPos.x) ? 1.0f : -1.0f;
            float playerSpeed = std::abs(pVel.x);
            koopa->kick(kickDirection, playerSpeed);
            m_stompCooldown = STOMP_COOLDOWN_TIME;
            std::cout << "Shell kicked!" << std::endl;
            break;
          } else {
            // Only take damage if touching the narrow "core" box
            player.takeDamage();
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
  // Dibujar suelo (tercera sección desde X=1216 con tilesets.png)
  window.draw(m_groundVertices3, &m_texture);

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

  // Dibujar plataformas sólidas con textura
  for (auto &plat : m_platforms) {
    window.draw(plat.vertices, &m_texture);
  }

  // Marcadores de debug para identificar límites de pantalla
  // Cada pantalla es de 800px de ancho, hay 8 pantallas
  static constexpr float SCREEN_WIDTH = 800.0f;
  static constexpr float SCREEN_HEIGHT = 600.0f;
  static constexpr int NUM_SCREENS = 8;
  static constexpr float MARKER_SIZE = 32.0f;

  sf::RectangleShape marker({MARKER_SIZE, MARKER_SIZE});

  // Colores diferentes para cada pantalla
  sf::Color screenColors[NUM_SCREENS] = {
      sf::Color::Red,         // Pantalla 1
      sf::Color::Green,       // Pantalla 2
      sf::Color::Blue,        // Pantalla 3
      sf::Color::Yellow,      // Pantalla 4
      sf::Color::Magenta,     // Pantalla 5
      sf::Color::Cyan,        // Pantalla 6
      sf::Color(255, 128, 0), // Pantalla 7 (Naranja)
      sf::Color(128, 0, 255)  // Pantalla 8 (Púrpura)
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