#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath> // Para std::abs
#include <iostream>

Player::Player(Physics& physics, float startX, float startY)
: m_physics(physics), m_width(32.0f), m_height(32.0f), m_canJump(false),
  m_animationTimer(0.0f), m_currentFrame(0), m_facingRight(true), m_state(State::Idle)
{
    // Load Texture
    if (!m_texture.loadFromFile("assets/images/mario_chiquito.png")) {
        std::cerr << "Error loading mario_chiquito.png" << std::endl;
        // Handle error? For now just continue, sprite will be white
    }
    m_sprite.setTexture(m_texture);
    
    // Set initial frame (Idle = 0)
    // Height 19, Origin Y 10 as per calibration V2
    m_sprite.setTextureRect(sf::IntRect(0, 0, 16, 19));
    
    // Center origin X (8), Align Y (10)
    m_sprite.setOrigin(16.0f / 2.0f, 10.0f);
    
    // Scale visual
    m_sprite.setScale(2.0f, 2.0f);


    // Definición del cuerpo (v3)
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){startX / Physics::SCALE, startY / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    // Crear el cuerpo usando el ID del mundo
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

    // Definir la forma (Caja)
    // Use slightly adjusting sizing for physics if we want it to match sprite better
    // But keeping original 32x48 for now to avoid breaking existing level collisions
    b2Polygon dynamicBox = b2MakeBox((m_width / 2.0f) / Physics::SCALE, (m_height / 2.0f) / Physics::SCALE);

    // Definir la "fixture" (propiedades físicas)
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;

    // Unir forma al cuerpo
    b2CreatePolygonShape(m_bodyId, &shapeDef, &dynamicBox);
}

void Player::handleInput()
{
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
    float desiredVel = 0.0f;
    const float moveSpeed = 4.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        desiredVel = -moveSpeed;
        m_facingRight = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        desiredVel = moveSpeed;
        m_facingRight = true;
    }

    // Determine State logic for animation
    // Check Jump
    if (!m_canJump) {
        m_state = State::Jumping;
    } else {
        // On ground
        if (std::abs(vel.x) > 0.1f) {
            // Check for braking (moving opposite to velocity)
            if ((desiredVel > 0 && vel.x < -0.5f) || (desiredVel < 0 && vel.x > 0.5f)) {
                 m_state = State::Braking;
            } else {
                 m_state = State::Running;
            }
        } else {
            m_state = State::Idle;
        }
    }

    // Physics Movement
    float velChange = desiredVel - vel.x;
    float impulse = b2Body_GetMass(m_bodyId) * velChange;
    b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){impulse, 0.0f}, true);

    // Salto
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) && m_canJump) {
        float jumpImpulse = -b2Body_GetMass(m_bodyId) * 6.0f;
        b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){0.0f, jumpImpulse}, true);
        m_canJump = false;
        m_state = State::Jumping;
    }
}

void Player::update(float dt)
{
    b2Vec2 pos = b2Body_GetPosition(m_bodyId);
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

    // Actualizar gráfico SFML
    m_sprite.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);
    
    // Scale flipping based on direction
    // Base scale is 2.0f
    if (m_facingRight) {
        m_sprite.setScale(2.0f, 2.0f);
    } else {
        m_sprite.setScale(-2.0f, 2.0f);
    }

    // Chequeo simple de suelo
    if (std::abs(vel.y) < 0.1f) m_canJump = true;
    else m_canJump = false;

    updateAnimation(dt);
}

void Player::updateAnimation(float dt) {
    const float frameTime = 0.1f; // Speed of animation
    int startFrame = 0;
    int numFrames = 1;

    // Configure frames based on state
    // Indices:
    // 0: Idle
    // 1: Brake
    // 2: Jump
    // 4,5,6: Run (User said 5,6,7. Assuming 1-based index in user request => 4,5,6)
    
    switch (m_state) {
        case State::Idle:
            startFrame = 0;
            numFrames = 1;
            break;
        case State::Braking:
            startFrame = 1;
            numFrames = 1;
            break;
        case State::Jumping:
            startFrame = 2; // User said 3rd sprite
            numFrames = 1;
            break;
        case State::Running:
            startFrame = 4; // User said 5,6,7 => Indices 4,5,6
            numFrames = 3;
            break;
    }

    m_animationTimer += dt;
    if (m_animationTimer >= frameTime) {
        m_animationTimer = 0.0f;
        m_currentFrame++;
        if (m_currentFrame >= numFrames) {
            m_currentFrame = 0;
        }
    }
    // If state changes, we might want to reset frame?
    // For simplicity, just wrap. But for single frames like Idle, m_currentFrame will be 0.
    
    // Calibración de sprites
    int textureIndex = startFrame + m_currentFrame;
    
    // Stride aproximado de 18 (17+1) o similar para evitar desalineación acumulada
    const int spriteStride = 17; // El usuario dijo 17 de ancho. Probemos ancho 16 y stride 17 o 17 y stride 18?
    // Si se ve el sprite ANTERIOR, es que estamos muy a la IZQUIERDA.
    // Index 5 * 17 = 85. Si vemos el anterior (4), el 4 acaba en 4*17+17 = 85.
    // Si vemos el anterior, significa que el sprite 5 empieza más a la DERECHA.
    // Probemos con un pequeño offset inicial o un stride mayor.
    // "Calcula bien a partir de los sprites"
    
    // Hipótesis: Los sprites están espaciados de forma irregular o el stride es mayor.
    // Probaremos Stride 18? (16 ancho + 2 gap? O 17 ancho + 1 gap?)
    // Y aumentamos altura para no cortar piernas.
    
    // Configuración ajustada visualmente según reporte
    int tileWidth = 16; 
    int tileHeight = 20; // Más alto para incluir piernas
    int gap = 1;         // Espacio entre sprites
    int offsetX = 0;     // Start offset

    // Recalculate based on specific adjustments requested
    // "Running cuts left, shows previous". imply we need to go Right. -> Larger stride or offset.
    // "Idle cuts legs" -> Taller height.

    // Let's try explicit coordinates if specific indices have different spacings, 
    // but assuming uniform grid first with corrected stride.
    // If 17 was causing overlap, let's try 18 per step?
    // User said "Approx 17x16".
    // 17 width + 1 gap = 18 stride?
    
    int left = textureIndex * (tileWidth + gap) + offsetX;
    // Si el índice es alto (5,6,7), el error se acumula. 
    // Si tileWidth=17 estricto solapaba, tal vez es 16 y gap 1? (Stride 17).
    // Pero yo usaba Stride 17 (17*idx) y solapaba.
    // Entonces el stride debe ser MAYOR.
    // Probemos Stride = 18? (5*18 = 90). Un salto de 5px a la derecha.
    
    // Sin embargo, usuario dice "corta la derecha del actual".
    // Si estamos a la izquierda, vemos (Final del Anterior) + (Principio del Actual) [y falta el final del actual].
    // Correcto.
    // Vamos a probar Stride 18.
    
    int finalStride = 18;
    left = textureIndex * finalStride;

    // Ajuste final según feedback:
    // - Height 24 traía colores de abajo -> Bajamos a 20.
    // - Levitaba -> Ajustamos origen para que los pies (abajo del sprite) coincidan con abajo del cuerpo.
    // Cuerpo (Physics) es 32x32. Centro a abajo = 16.
    // Sprite es Scale 2.0. Visualmente queremos 16 visual pixels desde el centro/origen hasta abajo.
    // 16 visual / 2.0 scale = 8 texture pixels.
    // Ajuste final V2:
    // - Height 20 tenía un pixel negro abajo. -> Bajamos a 19.
    // - Levitación:
    //   Si Feet están en y=18 (último pixel válido en altura 19), y queremos distancia visual 16px.
    //   (18 - OriginY) * 2.0 = 16.
    //   18 - OriginY = 8.
    //   OriginY = 10.
    
    m_sprite.setTextureRect(sf::IntRect(left, 0, 16, 19)); 
    m_sprite.setOrigin(16.0f / 2.0f, 10.0f);

}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(m_sprite);
}

sf::Vector2f Player::getPosition() const {
    return m_sprite.getPosition();
}