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

	m_playerConfig = { 32, 32, 10, 10, 10, 255, 0, 0, 4, 4, 2.5f };

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
	auto player = m_entities.addEntity("player");

	player->add<CTransform>(Vec2f(400.0f, 300.0f), Vec2f(0.0f, 0.0f), 0.0f);
	player->add<CShape>(m_playerConfig.SR, m_playerConfig.V,
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
		m_playerConfig.OT);
	player->add<CInput>();
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
	auto& playerTransform = player()->get<CTransform>();
	auto& playerInput = player()->get<CInput>();

	playerTransform.velocity = { 0.0, 0.0 };
	if (playerInput.up)
		playerTransform.velocity.y -= m_playerConfig.S;
	if (playerInput.left)
		playerTransform.velocity.x -= m_playerConfig.S;
	if (playerInput.down)
		playerTransform.velocity.y += m_playerConfig.S;
	if (playerInput.right)
		playerTransform.velocity.x += m_playerConfig.S;

	playerTransform.pos += playerTransform.velocity;
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
	playerShape.circle.setRotation(sf::degrees(playerTransform.angle));

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
			auto& playerInput = player()->get<CInput>();

			switch (keyPressed->scancode)
			{
			case sf::Keyboard::Scancode::Escape:
				m_running = false;
				break;
			case sf::Keyboard::Scancode::W:
				playerInput.up = true;
				break;
			case sf::Keyboard::Scancode::A:
				playerInput.left = true;
				break;
			case sf::Keyboard::Scancode::S:
				playerInput.down = true;
				break;
			case sf::Keyboard::Scancode::D:
				playerInput.right = true;
				break;
			default: break;
			}
		}

		if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			auto& playerInput = player()->get<CInput>();

			switch (keyReleased->scancode)
			{
			case sf::Keyboard::Scancode::Escape:
				m_running = false;
				break;
			case sf::Keyboard::Scancode::W:
				playerInput.up = false;
				break;
			case sf::Keyboard::Scancode::A:
				playerInput.left = false;
				break;
			case sf::Keyboard::Scancode::S:
				playerInput.down = false;
				break;
			case sf::Keyboard::Scancode::D:
				playerInput.right = false;
				break;
			default: break;
			}
		}

		if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (ImGui::GetIO().WantCaptureMouse) { continue; }

			auto& playerInput = player()->get<CInput>();

			switch (mousePressed->button)
			{
			case sf::Mouse::Button::Left:
				playerInput.shoot = true;
				break;
			case sf::Mouse::Button::Right:
				std::cout << "Right mouse button clicked" << std::endl;
				break;
			default: break;
			}
		}

		if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
		{
			if (ImGui::GetIO().WantCaptureMouse) { continue; }

			auto& playerInput = player()->get<CInput>();

			switch (mouseReleased->button)
			{
			case sf::Mouse::Button::Left:
				playerInput.shoot = false;
				break;
			case sf::Mouse::Button::Right:
				std::cout << "Right mouse button released" << std::endl;
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