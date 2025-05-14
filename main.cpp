#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "SFML works!");

    sf::CircleShape shape(200.0f);
    shape.setFillColor(sf::Color::Green);
    shape.setPosition({ 200, 200 });

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }
}