#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "SFML works!");
    ImGui::SFML::Init(window);

    bool drawCircle = true;
    float circleRadius = 200.0f;
    int circleSegments = 100;
    sf::CircleShape shape(circleRadius, circleSegments);
    shape.setFillColor(sf::Color::Green);
    shape.setPosition({ 200, 200 });

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();
        }
        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Window Title");
        ImGui::Text("Window text!");
        ImGui::Checkbox("Circle", &drawCircle);
        ImGui::SliderFloat("Radius", &circleRadius, 100.0f, 300.0f);
        ImGui::SliderInt("Segments", &circleSegments, 3, 150);
        ImGui::End();

        shape.setRadius(circleRadius);
        shape.setPointCount(circleSegments);

        window.clear();
        if (drawCircle)
            window.draw(shape);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}