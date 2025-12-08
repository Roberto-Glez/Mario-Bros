#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class GameWindow {
public:
    GameWindow(unsigned int width, unsigned int height, const std::string& title);
    ~GameWindow();

    void run(std::function<void(float)> update, std::function<void()> render);

    sf::RenderWindow& window();

private:
    sf::RenderWindow m_window;
};

#endif // GAMEWINDOW_HPP
