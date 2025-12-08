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

    auto update = [&](float dt) {
        // Step physics with fixed dt
        physics.step(dt);
        player.handleInput();
        player.update(dt);
    };

    auto render = [&]() {
        level.draw(window.window());
        player.draw(window.window());
    };

    window.run(update, render);

    return 0;
}
