#include "Game.h"

#include <iostream>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	m_window.create(sf::VideoMode({ 1280, 720 }), "Assignment 2");
	m_window.setFramerateLimit(60);

	ImGui::SFML::Init(m_window);

	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	spawnPlayer();
}

std::shared_ptr<Entity> Game::player()
{
	auto& player = m_entities.getEntities("player");
	assert(player.size() == 1);
	return player.front();
}

void Game::run()
{
	while (m_running)
	{
		m_entities.update();

		ImGui::SFML::Update(m_window, m_deltaClock.restart());

		sEnemySpawner();
		sMovement();
		sCollision();
		sUserInput();
		sGUI();
		sRender();

		m_currentFrame++;
	}
	m_window.close();
}

void Game::spawnPlayer()
{
	auto entity = m_entities.addEntity("player");

	entity->add<CTransform>(Vec2f(200.0f, 200.0f), Vec2f(1.0f, 1.0f), 0.0f);
	entity->add<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color(255, 0, 0), 4.0f);
	entity->add<CInput>();
}

void Game::spawnEnemy()
{
	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{

}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& target)
{

}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	
}

void Game::sMovement()
{
	auto& transform = player()->get<CTransform>();
	transform.pos += transform.velocity;
}

void Game::sLifespan()
{

}

void Game::sCollision()
{

}

void Game::sEnemySpawner()
{

}

void Game::sGUI()
{
	ImGui::Begin("Geometry Wars");
	ImGui::Text("Stuff Goes Here");
	ImGui::End();
}

void Game::sRender()
{
	m_window.clear();

	auto& playerTransform = player()->get<CTransform>();
	auto& playerShape = player()->get<CShape>();

	playerShape.circle.setPosition(playerTransform.pos);
	playerTransform.angle += 1.0f;
	playerShape.circle.setRotation(sf::radians(playerTransform.angle));

	m_window.draw(playerShape.circle);

	ImGui::SFML::Render(m_window);

	m_window.display();
}

void Game::sUserInput()
{
	while (const std::optional event = m_window.pollEvent())
	{
		ImGui::SFML::ProcessEvent(m_window, *event);
		
		if (event->is<sf::Event::Closed>())
		{
			m_running = false;
		}

		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			switch (keyPressed->scancode)
			{
			case sf::Keyboard::Scancode::Escape:
				m_running = false;
				break;
			default: break;
			}
		}

		if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			switch (keyReleased->scancode)
			{
			case sf::Keyboard::Scancode::Escape:
				m_running = false;
				break;
			default: break;
			}
		}

		if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (ImGui::GetIO().WantCaptureMouse) { continue; }

			switch (mousePressed->button)
			{
			case sf::Mouse::Button::Left:
				std::cout << "Left mouse button clicked" << std::endl;
				break;
			case sf::Mouse::Button::Right:
				std::cout << "Right mouse button clicked" << std::endl;
				break;
			default: break;
			}
		}
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
	m_running = !paused;
}