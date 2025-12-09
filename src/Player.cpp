#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <iostream>

Player::Player(Physics& physics, float startX, float startY)
: m_physics(physics), m_width(32.0f), m_height(48.0f), m_canJump(false),
  m_currentState(AnimationState::Idle), m_animationTimer(0.0f), m_currentFrame(0), m_facingRight(true)
{
    // Load Texture
    if (!m_texture.loadFromFile("assets/images/mario_spritesheet.png")) {
        std::cerr << "Error loading mario_spritesheet.png" << std::endl;
        // Fallback or error handling
    } else {
        std::cout << "Mario Texture Loaded: " << m_texture.getSize().x << "x" << m_texture.getSize().y << std::endl;
    }
    
    m_sprite.setTexture(m_texture);
    
    // Initial sprite setup
    // Assuming standard frame size based on hardcoded width/height or logic below
    // We'll set the texture rect in updateAnimation
    
    m_sprite.setOrigin(m_width / 2.0f, m_height / 2.0f); // Center origin

    // Definición del cuerpo (v3)
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){startX / Physics::SCALE, startY / Physics::SCALE};
    bodyDef.fixedRotation = true;
    
    // Crear el cuerpo usando el ID del mundo
    m_bodyId = b2CreateBody(m_physics.worldId(), &bodyDef);

    // Definir la forma (Caja)
    // Reduce collision box slightly for better feeling? Keeping same for now.
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

    float velChange = desiredVel - vel.x;
    float impulse = b2Body_GetMass(m_bodyId) * velChange;
    
    b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){impulse, 0.0f}, true);

    // Salto
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) && m_canJump) {
        float jumpImpulse = -b2Body_GetMass(m_bodyId) * 6.0f;
        b2Body_ApplyLinearImpulseToCenter(m_bodyId, (b2Vec2){0.0f, jumpImpulse}, true);
        m_canJump = false;
    }
}

void Player::update(float dt)
{
    b2Vec2 pos = b2Body_GetPosition(m_bodyId);
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

    // Actualizar gráfico SFML
    m_sprite.setPosition(pos.x * Physics::SCALE, pos.y * Physics::SCALE);

    // Chequeo simple de suelo
    if (std::abs(vel.y) < 0.1f) {
        
        // Landed?
        if (m_currentState == AnimationState::Jump) {
             m_currentState = AnimationState::Landing;
             m_animationTimer = 0.0f;
             m_currentFrame = 0; // Will map to Frame 4
             m_canJump = false; // Disable jump during landing anim? Or allow immediately?
             // Usually allow cancelling into jump/run, but for visual fidelity:
             m_canJump = true; // Allow jump
        } 
        else if (m_currentState == AnimationState::Landing) {
             // Let updateAnimation handle the transition out of Landing
             m_canJump = true;
        }
        else {
            // Already on ground (Idle or Run)
            m_canJump = true;
            if (std::abs(vel.x) > 0.1f) {
                m_currentState = AnimationState::Run;
            } else {
                m_currentState = AnimationState::Idle;
            }
        }
    } else {
        m_canJump = false;
        m_currentState = AnimationState::Jump;
    }

    updateAnimation(dt);
}

void Player::updateAnimation(float dt)
{
    m_animationTimer += dt;
    
    int numFrames = 1;
    float frameDuration = 0.1f;
    
    // Pixel constants
    int startX = 68;
    int BaseY = 50;   // Top of Idle Row (Row 2) - user provided
    int strideY = 23; // 18px height + 5px gap as derived from user data
    
    // Default dimensions
    int spriteW = 16;
    int spriteH = 18; // Default to 18 (Idle)
    
    int gapX = 4; // Gap between columns
    
    int rowOffset = 0;
    
    switch (m_currentState) {
        case AnimationState::Idle:
            numFrames = 2;
            rowOffset = 0; 
            spriteH = 18; // Tight crop for Idle
            frameDuration = 0.5f; 
            break;
        case AnimationState::Run:
            numFrames = 8;
            rowOffset = 1; // Row 3
            spriteH = 21; // Taller crop for Run (legs)
            frameDuration = 0.1f;
            break;
        case AnimationState::Jump:
            // Air state (frames 0, 1, 2, 3)
            numFrames = 4; // Excluding land frames
            rowOffset = 4;
            spriteH = 21; 
            {
                b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
                float vy = vel.y; 
                
                // Thresholds
                if (vy < -5.0f) m_currentFrame = 0; // Launch
                else if (vy < -2.0f) m_currentFrame = 1; // Rise
                else if (vy < 0.0f) m_currentFrame = 2; // Approaching Peak
                else m_currentFrame = 3; // Falling (Descent)
            }
            break;
        case AnimationState::Landing:
            // Sequence: Frame 4 -> Frame 5 -> Idle
            numFrames = 2;
            rowOffset = 4;
            spriteH = 21;
            frameDuration = 0.08f; // Fast landing
            
            // Logic handled in timer block below
            break;
    }

    if (m_currentState == AnimationState::Landing) {
        if (m_animationTimer >= frameDuration) {
            m_animationTimer = 0.0f;
            m_currentFrame++;
            if (m_currentFrame >= numFrames) {
                // Done landing, switch to Idle
                m_currentState = AnimationState::Idle;
                m_currentFrame = 0;
            }
        }
    } 
    else if (m_currentState != AnimationState::Jump) {
        if (m_animationTimer >= frameDuration) {
            m_animationTimer = 0.0f;
            m_currentFrame++;
            if (m_currentFrame >= numFrames) {
                m_currentFrame = 0; 
            }
        }
    }
    
    // Offsets for Jump curve (User specified curve on sheet)
    // Adjusted based on feedback (Head cropping on 1st frame, 2nd/3rd kept same)
    // Curve: {-6, -12, -18, -18, -12, 0}
    // Mapping:
    // Jump 0: -6
    // Jump 1: -12
    // Jump 2: -18
    // Jump 3: -18 (Descent uses Peak offset?) Or -12?
    // User said curve. Peak is -18. Descent should probably be -18 or -12.
    // Let's assume indices 3, 4, 5 map to -18, -12, 0 logic?
    // Let's stick to the array but indexed carefully.
    
    // Fix Landing levitation/clipping (Index 4): -12 (float) -> -18 (clip) -> -15 (try)
    int jumpOffsets[6] = {-6, -12, -18, -18, -15, 0}; 

    // Calculate Rect
    int rowY = BaseY + (rowOffset * strideY);
    
    // Apply curve offset only for Jump and Landing
    int yOffset = 0;
    if (m_currentState == AnimationState::Jump) {
         // Jump has frames 0, 1, 2, 3
         // Map to table: 0->0, 1->1, 2->2, 3->3
         if (m_currentFrame < 6) yOffset = jumpOffsets[m_currentFrame];
    }
    else if (m_currentState == AnimationState::Landing) {
        // Landing uses frames 0, 1 (mapped to user sprites 4, 5 i.e. indices 4, 5)
        // Offset mapping: 
        // Landing Frame 0 -> Sprite Index 4 -> Offset Index 4 (-12)
        // Landing Frame 1 -> Sprite Index 5 -> Offset Index 5 (0)
        int logicIndex = 4 + m_currentFrame;
        if (logicIndex < 6) yOffset = jumpOffsets[logicIndex];
        
        // VISUAL MAPPING FOR SPRITE INDEX
        // m_currentFrame 0 -> needs +4 to get Sprite 4
    }

    int currentY = rowY + yOffset;
    
    // Calculate X Index
    int xIndex = m_currentFrame;
    if (m_currentState == AnimationState::Landing) {
        xIndex += 4;
    }
    
    int currentX = startX + (xIndex * (spriteW + gapX));
    
    m_sprite.setTextureRect(sf::IntRect(currentX, currentY, spriteW, spriteH));

    // DYNAMIC ORIGIN FOR FEET ALIGNMENT
    // We want the feet (Bottom of sprite) to match Bottom of Physics Body.
    // Physics Half Height = 24.
    // Scale = 2.5.
    // Target Bottom (Unscaled) = 9.6f (24 / 2.5).
    // OriginY should be such that (Height - OriginY) = 9.6f.
    // OriginY = Height - 9.6f.
    
    float originY = spriteH - 9.6f;
    m_sprite.setOrigin(spriteW / 2.0f, originY);

    float scale = 2.5f;

    if (m_facingRight) {
        m_sprite.setScale(scale, scale);
    } else {
        m_sprite.setScale(-scale, scale);
    }
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(m_sprite);
}

sf::Vector2f Player::getPosition() const {
    return m_sprite.getPosition();
}