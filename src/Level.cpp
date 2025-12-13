#include "Level.hpp"
#include "Player.hpp"
#include <algorithm>
#include <iostream>

// ============================================================================
// DEBUG: Descomentar la siguiente línea para poner la meta cerca del inicio
//        y saltar rápidamente al siguiente nivel (para pruebas).
//        Comentar para volver al comportamiento normal.
// ============================================================================
// #define DEBUG_SKIP_LEVEL

Level::Level(Physics &physics, float width, float height, int levelNumber)
    : m_physics(physics), m_width(width), m_height(height),
      m_stompCooldown(0.0f), m_stompSound(m_stompSoundBuffer),
      m_powerupSound(m_powerupSoundBuffer), m_goalSound(m_goalSoundBuffer),
      m_levelNumber(levelNumber), m_bgSprite(m_bgTexture),
      m_cornerSprite(m_bgTexture) {
  // Load Textures
  if (!m_texture.loadFromFile("assets/images/tilesets.png")) {
    std::cerr << "Error loading tilesets.png" << std::endl;
  }
  if (!m_texture2.loadFromFile("assets/images/plataformas.png")) {
    std::cerr << "Error loading plataformas.png" << std::endl;
  }

  // Load Stomp Sound
  if (!m_stompSoundBuffer.loadFromFile("assets/music/aplastar.mp3")) {
    std::cerr << "Error loading aplastar.mp3" << std::endl;
  }

  // Load Powerup Sound
  if (!m_powerupSoundBuffer.loadFromFile("assets/music/powerup.wav")) {
    std::cerr << "Error loading powerup.wav" << std::endl;
  }

  // Load Goal Sound
  if (!m_goalSoundBuffer.loadFromFile("assets/music/goal_sound.wav")) {
    std::cerr << "Error loading goal_sound.wav" << std::endl;
  }

  // Initialize Goal
#ifdef DEBUG_SKIP_LEVEL
  // DEBUG: Meta cerca del inicio SOLO en nivel 1 para saltar rápidamente al
  // nivel 2
  if (m_levelNumber == 1) {
    m_goal.init(200.0f, height - 32.0f);
  } else {
    m_goal.init(LEVEL_WIDTH - 200.0f, height - 32.0f);
  }
#else
  // NORMAL: Meta al final del nivel
  m_goal.init(LEVEL_WIDTH - 200.0f, height - 32.0f);
#endif

  // Load Background Texture
  if (!m_bgTexture.loadFromFile("assets/images/background.png")) {
    std::cerr << "Error loading background.png" << std::endl;
  }
  // Load Trap Texture (for kill blocks)
  if (!m_trapTexture.loadFromFile("assets/images/trampa.png")) {
    std::cerr << "Error loading trampa.png" << std::endl;
  }
  // Ensure bgSprite uses the texture (for the main background loop)
  m_bgSprite.setTexture(m_bgTexture);
  m_bgSprite.setTextureRect(
      sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(1600, 750)));

  // Configure corner sprite (spray)
  // Region: (0, 748) is bottom-left. 74x74 up/right.
  // SFML IntRect(left, top, width, height) -> Top = 748 - 74 = 674.
  m_cornerSprite.setTexture(m_bgTexture);
  m_cornerSprite.setTextureRect(
      sf::IntRect(sf::Vector2i(0, 674), sf::Vector2i(74, 74)));
  m_cornerSprite.setPosition(sf::Vector2f(0.0f, 0.0f));

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

  // Tercera sección: desde X=1184 (tile 37) hasta 4800 (tile 150)
  static constexpr int THIRD_START_TILE = 1184 / DISPLAY_TILE_SIZE;  // Tile 37
  static constexpr int FOURTH_START_TILE = 4800 / DISPLAY_TILE_SIZE; // Tile 150

  int thirdNumTiles = FOURTH_START_TILE - THIRD_START_TILE;
  int fourthNumTiles = numTilesX - FOURTH_START_TILE;

  // Contar tiles normales (excluyendo las secciones alternativa, tercera y
  // cuarta)
  int normalTiles = numTilesX - ALT_NUM_TILES - thirdNumTiles - fourthNumTiles;

  m_groundVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices.resize(normalTiles * 6);

  // Segundo VertexArray para la sección alternativa
  m_groundVertices2.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices2.resize(ALT_NUM_TILES * 6);

  // Tercer VertexArray para la tercera sección
  m_groundVertices3.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices3.resize(thirdNumTiles * 6);

  // Cuarto VertexArray para la cuarta sección (X >= 4800)
  m_groundVertices4.setPrimitiveType(sf::PrimitiveType::Triangles);
  m_groundVertices4.resize(fourthNumTiles * 6);

  int texU = 80; // Sprite normal de tilesets.png
  int texV = 176;

  int texU2 = 272; // Sprite alternativo de plataformas.png
  int texV2 = 16;

  int texU3 = 240; // Sprite tercera sección de tilesets.png
  int texV3 = 96;

  int texU4 = 320; // Sprite cuarta sección de tilesets.png (320, 16)
  int texV4 = 16;

  int normalIdx = 0;
  int altIdx = 0;
  int thirdIdx = 0;
  int fourthIdx = 0;

  for (int i = 0; i < numTilesX; ++i) {
    float x = i * DISPLAY_TILE_SIZE;

    // Level 2 Gap: Skip tiles from pixel 160 to pixel (160 + 180 * 32) = 5920
    if (m_levelNumber == 2) {
      // Gap Starts at 160. Gap Width = 180 blocks * 32 = 5760. Gap Ends at
      // 5920.
      if (x >= 160.0f && x < (160.0f + 180.0f * 32.0f)) {
        continue; // Skip this tile
      }
    }
    float y = m_groundY;

    // Quad positions (32x32 en pantalla)
    sf::Vector2f p0(x, y);
    sf::Vector2f p1(x + DISPLAY_TILE_SIZE, y);
    sf::Vector2f p2(x + DISPLAY_TILE_SIZE, y + DISPLAY_TILE_SIZE);
    sf::Vector2f p3(x, y + DISPLAY_TILE_SIZE);

    // Determinar qué sección usar
    bool isAltSection = (i >= ALT_START_TILE && i < ALT_END_TILE);
    bool isThirdSection = (i >= THIRD_START_TILE && i < FOURTH_START_TILE);
    bool isFourthSection = (i >= FOURTH_START_TILE);

    if (isFourthSection) {
      // Usar cuarta textura (tilesets.png sprite 320,16)
      sf::Vertex *tri = &m_groundVertices4[fourthIdx * 6];

      sf::Vector2f t0((float)texU4, (float)texV4);
      sf::Vector2f t1((float)texU4 + TEX_TILE_SIZE, (float)texV4);
      sf::Vector2f t2((float)texU4 + TEX_TILE_SIZE,
                      (float)texV4 + TEX_TILE_SIZE);
      sf::Vector2f t3((float)texU4, (float)texV4 + TEX_TILE_SIZE);

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

      fourthIdx++;
    } else if (isThirdSection) {
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

      float u = (float)texU;
      float v = (float)texV;

      // Level 2 Start: Use Texture 4 (320, 16) for the first section (x < 160)
      if (m_levelNumber == 2 && x < 160.0f) {
        u = (float)texU4;
        v = (float)texV4;
      }

      sf::Vector2f t0(u, v);
      sf::Vector2f t1(u + TEX_TILE_SIZE, v);
      sf::Vector2f t2(u + TEX_TILE_SIZE, v + TEX_TILE_SIZE);
      sf::Vector2f t3(u, v + TEX_TILE_SIZE);

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
  // Cuerpo estático Box2D v3
  // Ground Physics
  if (m_levelNumber == 2) {
    // Level 2: Split ground with a gap [160, 5920]
    // Part 1: [0, 160]
    {
      b2BodyDef groundDef = b2DefaultBodyDef();
      float p1Width = 160.0f;
      float p1CenterX = p1Width / 2.0f;
      groundDef.position = (b2Vec2){p1CenterX / Physics::SCALE,
                                    (m_groundY + 16.0f) / Physics::SCALE};
      groundDef.type = b2_staticBody;
      b2BodyId s1 = b2CreateBody(m_physics.worldId(), &groundDef);
      b2Polygon box1 = b2MakeBox((p1Width / 2.0f) / Physics::SCALE,
                                 (16.0f) / Physics::SCALE);
      b2ShapeDef sd = b2DefaultShapeDef();
      b2CreatePolygonShape(s1, &sd, &box1);
    }
    // Part 2: [5920, LEVEL_WIDTH]
    // Start = 160 + 180*32 = 5920.
    // Width = 6400 - 5920 = 480.
    {
      float gapEnd = 160.0f + 180.0f * 32.0f; // 5920
      float p2Width = LEVEL_WIDTH - gapEnd;   // 480
      if (p2Width > 0) {
        b2BodyDef groundDef = b2DefaultBodyDef();
        float p2CenterX = gapEnd + p2Width / 2.0f;
        groundDef.position = (b2Vec2){p2CenterX / Physics::SCALE,
                                      (m_groundY + 16.0f) / Physics::SCALE};
        groundDef.type = b2_staticBody;
        b2BodyId s2 = b2CreateBody(m_physics.worldId(), &groundDef);
        b2Polygon box2 = b2MakeBox((p2Width / 2.0f) / Physics::SCALE,
                                   (16.0f) / Physics::SCALE);
        b2ShapeDef sd = b2DefaultShapeDef();
        b2CreatePolygonShape(s2, &sd, &box2);

        // Assign main ID to one of them (or leave it unused if not critical,
        // but let's assign the end piece)
        m_groundBodyId = s2;
      }
    }
  } else {
    // Original Logic (Level 1 / Default)
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
  }

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

  // ========== LEVEL-SPECIFIC CONTENT ==========
  if (m_levelNumber == 1) {
    // Bloques distribuidos a lo largo del nivel extendido (alineados a la
    // cuadrícula) Altura: 3 bloques de espacio vacío = 96px. Centro del bloque
    // = 96 + 16 = 112px arriba del suelo.
    float blockY = m_groundY - 112.0f;
    // X alineados a centros de tiles (N * 32 + 16)
    m_blocks.emplace_back(m_physics, 1808.0f, blockY); // Tile 56
    m_blocks.emplace_back(m_physics, 2096.0f, blockY); // Tile 65
    m_blocks.emplace_back(m_physics, 2416.0f, blockY); // Tile 75
    m_blocks.emplace_back(m_physics, 2704.0f, blockY); // Tile 84
    m_blocks.emplace_back(m_physics, 2992.0f, blockY); // Tile 93

    // Nuevos enemigos solicitados
    // Goomba en X=320 eliminado
    // m_enemies.push_back(
    //     std::make_unique<Goomba>(m_physics, 320.0f, m_groundY - 96.0f));

    // Goomba en X=416, nivel del suelo
    m_enemies.push_back(std::make_unique<Goomba>(m_physics, 416.0f, m_groundY));

    // Koopa en X=578, nivel del suelo
    m_enemies.push_back(std::make_unique<Koopa>(m_physics, 578.0f, m_groundY));

    // Goomba en X=864, nivel del suelo
    m_enemies.push_back(std::make_unique<Goomba>(m_physics, 864.0f, m_groundY));

    // Koopa en X=1056, nivel del suelo
    m_enemies.push_back(std::make_unique<Koopa>(m_physics, 1056.0f, m_groundY));

    // Goomba en X=1504, nivel del suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 1504.0f, m_groundY));

    // Goomba en X=1664, nivel del suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 1664.0f, m_groundY));

    // Goomba en X=1824, nivel del suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 1824.0f, m_groundY));

    // Goomba en X=2016, altura 5 bloques (160px) sobre el suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2016.0f, m_groundY - 160.0f));

    // Goomba en X=2112, altura 8 bloques (256px) sobre el suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2112.0f, m_groundY - 256.0f));

    // Goomba en X=2208, altura 8 bloques (256px) sobre el suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2208.0f, m_groundY - 256.0f));

    // Goomba en X=3104 eliminado
    // m_enemies.push_back(std::make_unique<Goomba>(m_physics, 3104.0f,
    // m_groundY));

    // Goomba en X=3264 cambiado a Koopa
    m_enemies.push_back(std::make_unique<Koopa>(m_physics, 3264.0f, m_groundY));

    // Koopa en X=3424, nivel del suelo
    m_enemies.push_back(std::make_unique<Koopa>(m_physics, 3424.0f, m_groundY));

    // Koopa en X=3840, nivel del suelo
    m_enemies.push_back(std::make_unique<Koopa>(m_physics, 3840.0f, m_groundY));

    // Koopa en X=3936, nivel del suelo
    m_enemies.push_back(std::make_unique<Koopa>(m_physics, 3936.0f, m_groundY));

    // Goomba en X=4288, nivel del suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 4288.0f, m_groundY));

    // Goomba en X=4448, nivel del suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 4448.0f, m_groundY));

    // Goomba en X=4704, nivel del suelo
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 4704.0f, m_groundY));

    // Plataforma con textura en X=320, 96px arriba del suelo (3 bloques de
    // ancho)
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
      plat.x = 2080.0f; // Bloque horizontal 65 (65 * 32 = 2080)
      plat.y =
          m_groundY - 256.0f; // 8 bloques arriba del suelo (8 * 32 = 256px)
      plat.width = 192.0f;    // 6 bloques de ancho (6 * 32 = 192px)
      plat.height = 256.0f;   // 8 bloques de altura (8 * 32 = 256px)

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
      plat.x = 2688.0f; // Pixel 2688 horizontal
      plat.y =
          m_groundY - 160.0f; // 5 bloques arriba del suelo (5 * 32 = 160px)
      plat.width = 224.0f;    // 7 bloques de ancho (7 * 32 = 224px)
      plat.height = 160.0f;   // 5 bloques de altura (5 * 32 = 160px)

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

    // Isla flotante en X=3040, separada 160px del suelo (10 bloques × 3
    // bloques)
    {
      Platform plat;
      plat.x = 3040.0f; // Pixel 3040 horizontal
      plat.y =
          m_groundY - 256.0f; // 160px de separación + 96px de altura = 256px
      plat.width = 320.0f;    // 10 bloques de ancho (10 * 32 = 320px)
      plat.height = 96.0f;    // 3 bloques de altura (3 * 32 = 96px)

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
      plat.x = 3520.0f; // Pixel 3520 horizontal
      plat.y =
          m_groundY - 352.0f; // 11 bloques desde el suelo (11 * 32 = 352px)
      plat.width = 256.0f;    // 8 bloques de ancho (8 * 32 = 256px)
      plat.height = 352.0f;   // 11 bloques de altura (11 * 32 = 352px)

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

    // Sección izquierda extrema de la plataforma (antes del pasaje inferior)
    // X=4000, ancho=64px (2 bloques), altura=14 bloques
    {
      Platform plat;
      plat.x = 4000.0f;
      plat.y = m_groundY - 448.0f;
      plat.width = 64.0f;   // 2 bloques de ancho
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

    // Plataforma nueva en X=4064, distancia 2 bloques del suelo (gap 64px)
    // Ancho 2 bloques (64px), Alto 1 bloque (32px)
    {
      Platform plat;
      plat.x = 4064.0f;
      plat.y = m_groundY - 96.0f;
      plat.width = 64.0f;
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

    // Plataforma nueva en X=4224, distancia 5 bloques del suelo (gap 160px)
    // Ancho 2 bloques (64px), Alto 1 bloque (32px)
    {
      Platform plat;
      plat.x = 4224.0f;
      plat.y =
          m_groundY - 192.0f; // 160px gap + 32px height = 192px from ground
      plat.width = 64.0f;
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

    // Plataforma nueva en X=4736, distancia 9 bloques del suelo (gap 288px)
    // Ancho 2 bloques (64px), Alto 1 bloque (32px)
    {
      Platform plat;
      plat.x = 4736.0f;
      plat.y =
          m_groundY - 320.0f; // 288px gap + 32px height = 320px from ground
      plat.width = 64.0f;
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

    // Plataforma nueva en X=4640, distancia 12 bloques del suelo (gap 384px)
    // Ancho 1 bloque (32px), Alto 1 bloque (32px)
    {
      Platform plat;
      plat.x = 4640.0f;
      plat.y =
          m_groundY - 416.0f; // 384px gap + 32px height = 416px from ground
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

    // Sección arriba del pasaje inferior (techo del pasaje a nivel de suelo)
    // X=4064, ancho=224px (7 bloques), altura=2 bloques (64px) en el tope
    {
      Platform plat;
      plat.x = 4064.0f;
      plat.y = m_groundY - 448.0f; // Desde el tope
      plat.width = 224.0f;         // 7 bloques de ancho
      plat.height = 64.0f;         // 2 bloques de altura (14 - 12 = 2)

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

    // Sección debajo del pasaje superior (2 bloques de altura, dejando pasaje
    // de 5 bloques a nivel de suelo) X=4288, ancho=352px (11 bloques), altura=2
    // bloques (64px)
    {
      Platform plat;
      plat.x = 4288.0f;
      plat.y =
          m_groundY - 224.0f; // A 7 bloques desde el suelo (encima del pasaje)
      plat.width = 352.0f;    // 11 bloques de ancho
      plat.height = 64.0f;    // 2 bloques de altura (7 - 5 = 2)

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
      plat.y = m_groundY -
               960.0f;      // 5 bloques separación + 25 bloques altura = 960px
      plat.width = 800.0f;  // 25 bloques de ancho (25 * 32 = 800px)
      plat.height = 800.0f; // 25 bloques de altura (25 * 32 = 800px)

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



    // Background texture loading and setup moved to top of constructor to avoid
    // duplicates and errors.

    // Corner Sprite (Spray) initialization
    // Source: Bottom-left (0, 748) -> 74x74 area upwards and right
    // Rect: x=0, y=674 (748-74), w=74, h=74
    // Duplicate m_cornerSprite initialization removed (moved to top)

    // Bloques asesinos desde X=2272 hasta X=2688, al ras del suelo
    for (float x = 2272.0f; x < 2688.0f; x += 32.0f) {
      // Use the explicit constructor we added to KillBlock
      KillBlock kBlock(m_trapTexture);
      // REDUCED COLLISION: 16px width
      kBlock.x = x + 8.0f; // Offset collision
      kBlock.y = m_groundY - 32.0f;
      kBlock.width = 16.0f;
      kBlock.height = 32.0f;

      // Physics Body (Static)
      b2BodyDef bodyDef = b2DefaultBodyDef();
      bodyDef.type = b2_staticBody;
      bodyDef.position =
          (b2Vec2){(kBlock.x + kBlock.width / 2.0f) / Physics::SCALE,
                   (kBlock.y + kBlock.height / 2.0f) / Physics::SCALE};
      kBlock.bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

      b2Polygon box = b2MakeBox((kBlock.width / 2.0f) / Physics::SCALE,
                                (kBlock.height / 2.0f) / Physics::SCALE);
      b2ShapeDef shapeDef = b2DefaultShapeDef();
      b2CreatePolygonShape(kBlock.bodyId, &shapeDef, &box);

      // Visual Shape (Optional now, but kept for debug or fallback)
      kBlock.shape.setSize({kBlock.width, kBlock.height});
      kBlock.shape.setPosition({kBlock.x, kBlock.y});
      kBlock.shape.setFillColor(sf::Color::Red);

      // Sprite Configuration
      // Texture already set in constructor
      // Crop: from (26, 286) with size 434x173
      kBlock.sprite.setTextureRect(sf::IntRect({26, 286}, {434, 173}));

      // Scale to fit 32x32 block
      // Desired / Original
      float scaleX = 32.0f / 434.0f;
      float scaleY = 32.0f / 173.0f;
      kBlock.sprite.setScale({scaleX, scaleY});

      // VISUAL POSITION: Original x (centered visually)
      kBlock.sprite.setPosition({x, kBlock.y});

      m_killBlocks.push_back(kBlock);
    }

    // Grupos de bloques trampa (Varios grupos configurados)
    struct TrapGroup {
      float startX;
      float heightBlocks; // Altura en bloques desde el suelo
      int count;
    };

    std::vector<TrapGroup> groups = {
        // Grupos originales elevados (15 bloques de altura)
        {4128.0f, 15.0f, 3},
        {4320.0f, 15.0f, 3},
        {4512.0f, 15.0f, 3},

        // Nuevos grupos solicitados (8 bloques de altura)
        {4352.0f, 8.0f, 1},
        {4544.0f, 8.0f, 1},

        // Nuevos grupos en el suelo (1 bloque de altura)
        {4352.0f, 1.0f, 1},
        {4544.0f, 1.0f, 1}};

    for (const auto &group : groups) {
      float groupY = m_groundY - (group.heightBlocks * 32.0f);

      for (int i = 0; i < group.count; ++i) {
        float x = group.startX + (i * 32.0f);

        KillBlock kBlock(m_trapTexture);

        // REDUCED COLLISION: 16px width
        kBlock.x = x + 8.0f; // Offset collision
        kBlock.y = groupY;
        kBlock.width = 16.0f;
        kBlock.height = 32.0f;

        // Physics Body (Static)
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position =
            (b2Vec2){(kBlock.x + kBlock.width / 2.0f) / Physics::SCALE,
                     (kBlock.y + kBlock.height / 2.0f) / Physics::SCALE};
        kBlock.bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

        b2Polygon box = b2MakeBox((kBlock.width / 2.0f) / Physics::SCALE,
                                  (kBlock.height / 2.0f) / Physics::SCALE);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(kBlock.bodyId, &shapeDef, &box);

        // Visual Shape
        kBlock.shape.setSize({kBlock.width, kBlock.height});
        kBlock.shape.setPosition({kBlock.x, kBlock.y});
        kBlock.shape.setFillColor(sf::Color::Red);

        // Sprite Configuration
        kBlock.sprite.setTextureRect(sf::IntRect({26, 286}, {434, 173}));

        float scaleX = 32.0f / 434.0f;
        float scaleY = 32.0f / 173.0f;
        kBlock.sprite.setScale({scaleX, scaleY});

        // VISUAL POSITION: Original x (centered visually)
        kBlock.sprite.setPosition({x, kBlock.y});

        m_killBlocks.push_back(kBlock);
      }
    }
  } // End of level 1 content
  else if (m_levelNumber == 2) {
    // ========== LEVEL 2: PLATFORMS WITH TEXTURE ==========
    // Ground and physics bodies were already created above
    // All platforms use tilesets.png sprite at (160,96) 16x16 scaled to 32x32

    // Helper lambda to create a textured platform
    auto createTexturedPlatform = [&](float pX, float pY, float pW, float pH) {
      Platform plat;
      plat.x = pX;
      plat.y = pY;
      plat.width = pW;
      plat.height = pH;

      // Physics Body
      b2BodyDef platBodyDef = b2DefaultBodyDef();
      platBodyDef.type = b2_staticBody;
      platBodyDef.position = (b2Vec2){(pX + pW / 2.0f) / Physics::SCALE,
                                      (pY + pH / 2.0f) / Physics::SCALE};
      plat.bodyId = b2CreateBody(m_physics.worldId(), &platBodyDef);

      b2Polygon platBox =
          b2MakeBox((pW / 2.0f) / Physics::SCALE, (pH / 2.0f) / Physics::SCALE);
      b2ShapeDef platShapeDef = b2DefaultShapeDef();
      b2CreatePolygonShape(plat.bodyId, &platShapeDef, &platBox);

      // Visual with tiles from tilesets.png (160,96) 16x16 scaled to 32x32
      int tilesX = static_cast<int>(pW / 32.0f);
      int tilesY = static_cast<int>(pH / 32.0f);
      plat.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
      plat.vertices.resize(tilesX * tilesY * 6);

      for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
          int idx = (ty * tilesX + tx) * 6;
          float px = pX + tx * 32.0f;
          float py = pY + ty * 32.0f;

          // Quad positions (32x32 on screen)
          sf::Vector2f p0(px, py);
          sf::Vector2f p1(px + 32.0f, py);
          sf::Vector2f p2(px + 32.0f, py + 32.0f);
          sf::Vector2f p3(px, py + 32.0f);

          // Texture coords (16x16 in image at 160,96)
          sf::Vector2f t0(160.0f, 96.0f);
          sf::Vector2f t1(176.0f, 96.0f);
          sf::Vector2f t2(176.0f, 112.0f);
          sf::Vector2f t3(160.0f, 112.0f);

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
    };

    // Power-up blocks for Level 2
    // Height: 14 blocks = 448px. Block center = 448 + 16 = 464px above ground
    float blockY = m_groundY - 464.0f;
    m_blocks.emplace_back(m_physics, 1056.0f, blockY); // Block at X=1056
    m_blocks.emplace_back(m_physics, 2016.0f, blockY); // Block at X=2016

    // Goombas for Level 2
    // Group 1: X=1568, 8 blocks high (256px), 4 goombas every 2 blocks (64px)
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 1568.0f, m_groundY - 256.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 1632.0f, m_groundY - 256.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 1696.0f, m_groundY - 256.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 1760.0f, m_groundY - 256.0f));

    // Group 2: X=2240, 13 blocks high (416px), 5 goombas every 2 blocks (64px)
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2240.0f, m_groundY - 416.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2304.0f, m_groundY - 416.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2368.0f, m_groundY - 416.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2432.0f, m_groundY - 416.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2496.0f, m_groundY - 416.0f));

    // Group 3: X=2784, 7 blocks high (224px), 5 goombas every 5 blocks (160px)
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2784.0f, m_groundY - 224.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 2944.0f, m_groundY - 224.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 3104.0f, m_groundY - 224.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 3264.0f, m_groundY - 224.0f));
    m_enemies.push_back(
        std::make_unique<Goomba>(m_physics, 3424.0f, m_groundY - 224.0f));

    // Kill Blocks for Level 2
    // Helper lambda to create a kill block
    auto createKillBlock = [&](float x, float heightBlocks) {
      float blockY = m_groundY - (heightBlocks * 32.0f);

      KillBlock kBlock(m_trapTexture);

      kBlock.x = x + 8.0f; // Offset collision
      kBlock.y = blockY;
      kBlock.width = 16.0f;
      kBlock.height = 32.0f;

      // Physics Body (Static)
      b2BodyDef bodyDef = b2DefaultBodyDef();
      bodyDef.type = b2_staticBody;
      bodyDef.position =
          (b2Vec2){(kBlock.x + kBlock.width / 2.0f) / Physics::SCALE,
                   (kBlock.y + kBlock.height / 2.0f) / Physics::SCALE};
      kBlock.bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

      b2Polygon box = b2MakeBox((kBlock.width / 2.0f) / Physics::SCALE,
                                (kBlock.height / 2.0f) / Physics::SCALE);
      b2ShapeDef shapeDef = b2DefaultShapeDef();
      b2CreatePolygonShape(kBlock.bodyId, &shapeDef, &box);

      // Visual Shape
      kBlock.shape.setSize({kBlock.width, kBlock.height});
      kBlock.shape.setPosition({kBlock.x, kBlock.y});
      kBlock.shape.setFillColor(sf::Color::Red);

      // Sprite Configuration
      kBlock.sprite.setTextureRect(sf::IntRect({26, 286}, {434, 173}));
      float scaleX = 32.0f / 434.0f;
      float scaleY = 32.0f / 173.0f;
      kBlock.sprite.setScale({scaleX, scaleY});
      kBlock.sprite.setPosition({x, kBlock.y});

      m_killBlocks.push_back(kBlock);
    };

    // Group 1: X=2838, 7 blocks high (224px), 5 kill blocks every 5 blocks
    // (160px)
    createKillBlock(2838.0f, 7.0f);
    createKillBlock(2998.0f, 7.0f);
    createKillBlock(3158.0f, 7.0f);
    createKillBlock(3318.0f, 7.0f);
    createKillBlock(3478.0f, 7.0f);

    // Group 2: X=4416, 10 blocks high (320px)
    // First pair (2 blocks)
    createKillBlock(4416.0f, 10.0f);
    createKillBlock(4448.0f, 10.0f);
    // Second pair moved 1 block left (from 4640 to 4608)
    createKillBlock(4608.0f, 10.0f);
    createKillBlock(4640.0f, 10.0f);

    // Group 3: X=5120, 4 blocks high (128px)
    // First triplet (3 blocks)
    createKillBlock(5120.0f, 4.0f);
    createKillBlock(5152.0f, 4.0f);
    createKillBlock(5184.0f, 4.0f);
    // Second triplet starts at 5216 + 160 = 5376 (5 blocks from end of first
    // triplet)
    createKillBlock(5376.0f, 4.0f);
    createKillBlock(5408.0f, 4.0f);
    createKillBlock(5440.0f, 4.0f);

    // PLATFORM 1: X=225, 3 blocks high, 3 blocks wide
    createTexturedPlatform(225.0f, m_groundY - 96.0f, 96.0f, 32.0f);

    // PLATFORM 2: X=448, 6 blocks high, 3 blocks wide
    createTexturedPlatform(448.0f, m_groundY - 192.0f, 96.0f, 32.0f);

    // PLATFORM 3: X=640, 9 blocks high, 3 blocks wide
    createTexturedPlatform(640.0f, m_groundY - 288.0f, 96.0f, 32.0f);

    // PLATFORM 4: X=832, 13 blocks high, 3 blocks wide
    createTexturedPlatform(832.0f, m_groundY - 416.0f, 96.0f, 32.0f);

    // PLATFORM 5: X=1024, 11 blocks high, 3 blocks wide
    createTexturedPlatform(1024.0f, m_groundY - 352.0f, 96.0f, 32.0f);

    // PLATFORM 6: X=1280, 14 blocks high, 3 blocks wide
    createTexturedPlatform(1280.0f, m_groundY - 448.0f, 96.0f, 32.0f);

    // PLATFORM 7: X=1504, 8 blocks high, 1 block wide
    createTexturedPlatform(1504.0f, m_groundY - 256.0f, 32.0f, 32.0f);

    // PLATFORM 8: X=1536, 7 blocks high, 8 blocks wide
    createTexturedPlatform(1536.0f, m_groundY - 224.0f, 256.0f, 32.0f);

    // PLATFORM 9: X=1792, 8 blocks high, 3 blocks wide
    createTexturedPlatform(1792.0f, m_groundY - 256.0f, 96.0f, 32.0f);

    // PLATFORM 10: X=1984, 11 blocks high, 3 blocks wide
    createTexturedPlatform(1984.0f, m_groundY - 352.0f, 96.0f, 32.0f);

    // PLATFORM 11: X=2144, 13 blocks high, 2 blocks wide
    createTexturedPlatform(2144.0f, m_groundY - 416.0f, 64.0f, 32.0f);

    // PLATFORM 12: X=2208, 12 blocks high, 10 blocks wide
    createTexturedPlatform(2208.0f, m_groundY - 384.0f, 320.0f, 32.0f);

    // PLATFORM 13: X=2528, 13 blocks high, 3 blocks wide
    createTexturedPlatform(2528.0f, m_groundY - 416.0f, 96.0f, 32.0f);

    // PLATFORM 14: X=2720, 7 blocks high, 1 block wide
    createTexturedPlatform(2720.0f, m_groundY - 224.0f, 32.0f, 32.0f);

    // PLATFORM 15: X=2752, 6 blocks high, 27 blocks wide
    createTexturedPlatform(2752.0f, m_groundY - 192.0f, 864.0f, 32.0f);

    // PLATFORM 16: X=3616, 7 blocks high, 1 block wide
    createTexturedPlatform(3616.0f, m_groundY - 224.0f, 32.0f, 32.0f);

    // PLATFORM 17: X=3744, 9 blocks high, 4 blocks wide
    createTexturedPlatform(3744.0f, m_groundY - 288.0f, 128.0f, 32.0f);

    // PLATFORM 18: X=3968, 12 blocks high, 4 blocks wide
    createTexturedPlatform(3968.0f, m_groundY - 384.0f, 128.0f, 32.0f);

    // PLATFORM 19: X=4352, 9 blocks high, 12 blocks wide
    createTexturedPlatform(4352.0f, m_groundY - 288.0f, 384.0f, 32.0f);

    // PLATFORM 20: X=5024, 3 blocks high, 16 blocks wide
    createTexturedPlatform(5024.0f, m_groundY - 96.0f, 512.0f, 32.0f);
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

  // Update Goal animation
  m_goal.update(dt);
}

void Level::checkCollisions(Player &player) {
  // Check Head collision with Blocks
  sf::Vector2f pPos = player.getPosition();
  sf::FloatRect pBounds = player.getBounds();

  // Define a small sensor box above the player's head
  sf::FloatRect headRect({pPos.x - 5, pBounds.position.y - 5}, {10, 10});

  // Check KillBlock collisions
  for (const auto &kBlock : m_killBlocks) {
    sf::FloatRect blockBounds({kBlock.x, kBlock.y},
                              {kBlock.width, kBlock.height});
    // Slightly expand block bounds to ensure touch is registered
    if (player.getBounds().findIntersection(blockBounds)) {
      player.die();
    }
  }

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
        m_powerupSound.play(); // Play powerup sound

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
      // Sprite is ~21 wide. 24 extends slightly beyond visual to punish side
      // touches.
      float dmgW = 24.0f;
      float dmgH = 18.0f; // Body height
      sf::FloatRect damageBox({enemyPos.x - dmgW / 2.0f, enemyPos.y - dmgH},
                              {dmgW, dmgH});

      // Stomp Hitbox: Very Narrow (12) and Top Only.
      // You must land ALMOST CENTERED on the head.
      // If you hit the "shoulder" (side-top), you miss this box and hit
      // damageBox instead.
      float stompW = 12.0f;
      float stompH = 6.0f;
      sf::FloatRect stompBox(
          {enemyPos.x - stompW / 2.0f, enemyPos.y - dmgH - stompH},
          {stompW, stompH});

      // Players velocity
      b2Vec2 pVel = player.getVelocity();
      // Must be falling significantly (> 1.0f) to count as a stomp
      bool isFalling = pVel.y > 1.0f;

      // Check Stomp Intersection First
      if (isFalling && pBounds.findIntersection(stompBox)) {
        enemy->stomp();
        player.bounce();
        m_stompCooldown = STOMP_COOLDOWN_TIME;
        m_stompSound.play(); // Play stomp sound
        std::cout << "Enemy stomped (Strict Custom Hitbox)!" << std::endl;
        break;
      }
      // Check Damage Intersection Second
      else if (pBounds.findIntersection(damageBox)) {
        Koopa *koopa = dynamic_cast<Koopa *>(enemy.get());
        if (koopa && koopa->isIdleShell()) {
          float kickDirection =
              (player.getPosition().x < enemyPos.x) ? 1.0f : -1.0f;
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

  // Check Goal collision
  if (!m_goal.isTriggered()) {
    if (player.getPosition().x >= m_goal.getX()) {
      m_goal.trigger();
      m_goalSound.play();
      std::cout << "Goal reached!" << std::endl;
    }
  }
}

void Level::draw(sf::RenderWindow &window) {
  // Dibujar Fondo (Repetir 4 veces para cubrir 6400px de ancho)
  for (int i = 0; i < 4; ++i) {
    m_bgSprite.setPosition(sf::Vector2f(i * 1600.0f, m_groundY - 750.0f));
    window.draw(m_bgSprite);
  }

  // Draw Corner Sprite (Spray)
  window.draw(m_cornerSprite);
  // CHECKER_SIZE removed

  // Calcular cuántas filas desde el suelo hacia arriba
  // CHECKERED BACKGROUND REMOVED

  // Dibujar suelo (sección normal con tilesets.png)
  window.draw(m_groundVertices, &m_texture);
  // Dibujar suelo (sección alternativa con plataformas.png)
  window.draw(m_groundVertices2, &m_texture2);
  // Dibujar suelo (tercera sección desde X=1216 con tilesets.png)
  window.draw(m_groundVertices3, &m_texture);
  // Dibujar suelo (cuarta sección desde X=4800 con tilesets.png)
  window.draw(m_groundVertices4, &m_texture);

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

  // Draw Colored Platforms
  for (const auto &plat : m_coloredPlatforms) {
    window.draw(plat);
  }

  // Dibujar Bloques Asesinos
  for (const auto &kBlock : m_killBlocks) {
    window.draw(kBlock.sprite);
  }

  // SCREEN BOUNDARY MARKERS REMOVED

  // Draw Goal
  m_goal.draw(window);
}

void Level::spawnFireball(float x, float y, float direction) {
  m_fireballs.push_back(std::make_unique<Fireball>(m_physics, x, y, direction));
}

float Level::groundY() const { return m_groundY; }