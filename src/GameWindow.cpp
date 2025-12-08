#include "GameWindow.hpp"
#include <SFML/Window/Event.hpp>
#include <chrono>

GameWindow::GameWindow(unsigned int width, unsigned int height, const std::string& title)
: m_window(sf::VideoMode(width, height), title)
{
    m_window.setFramerateLimit(60);
}

GameWindow::~GameWindow() {}

void GameWindow::run(std::function<void(float)> update, std::function<void()> render)
{
    sf::Clock clock;
    while (m_window.isOpen()) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) m_window.close();
        }

        float dt = clock.restart().asSeconds();

        update(dt);

        m_window.clear(sf::Color(100, 149, 237));
        render();
        m_window.display();
    }
}

sf::RenderWindow& GameWindow::window()
{
    return m_window;
}
