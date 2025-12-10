#include "GameWindow.hpp"
#include "Physics.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include <memory>
#include <iostream>

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;

    GameWindow window(WIDTH, HEIGHT, "Mario - Demo (SFML + Box2D)");

    Physics physics;

    Level level(physics, (float)WIDTH, (float)HEIGHT);

    Player player(physics, 100.0f, 400.0f);

    // Configurar cámara
    sf::View camera(sf::FloatRect(0, 0, (float)WIDTH, (float)HEIGHT));

    auto update = [&](float dt) {
        // Step physics with fixed dt
        physics.step(dt);
        player.handleInput(dt); // Pass dt for acceleration timer
        player.update(dt);
        
        // Actualizar cámara
        // Seguir en X
        float camX = player.getPosition().x;
        
        // Seguir en Y con restricción (Clamp)
        // El suelo está abajo (Y grande). Queremos que la cámara NO baje más allá del suelo.
        // Centro de cámara máximo = HEIGHT / 2.0f (Para que el borde inferior esté en HEIGHT)
        float maxCamY = (float)HEIGHT / 2.0f;
        float camY = std::min(player.getPosition().y, maxCamY);

        camera.setCenter(camX, camY);
    };

    auto render = [&]() {
        // Aplicar vista a la ventana
        window.window().setView(camera);
        
        level.draw(window.window());
        player.draw(window.window());
    };

    window.run(update, render);

    return 0;
}
