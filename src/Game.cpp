#include "Game.h"

#include <iostream>
#include <random>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	m_window.create(sf::VideoMode({ m_windowConfig.uW, m_windowConfig.uH }), "Assignment 2");
	m_window.setFramerateLimit(m_windowConfig.FR);

	ImGui::SFML::Init(m_window);

	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	// int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S
	m_playerConfig = { 32, 32, 100, 100, 100, 250, 250, 250, 4, 6, 5.0f };
	// int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S
	m_bulletConfig = { 6, 6, 250, 250, 250, 250, 250, 250, 2, 12, 100, 10.0f };
	// int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX
	m_enemyConfig = { 40, 40, 250, 10, 10, 3, 4, 8, 60, 100, 2.0f, 8.0f };

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
		sLifespan();
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

	player->add<CTransform>(Vec2f(m_windowConfig.fW, m_windowConfig.fH) / 2,
		Vec2f(0.0f, 0.0f), 0.0f);
	player->add<CShape>(m_playerConfig.SR, m_playerConfig.V,
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
		m_playerConfig.OT);
	player->add<CCollision>(m_playerConfig.CR);
	player->add<CInput>();
}

void Game::spawnEnemy()
{
	// Create random device and seed generator
	std::random_device rd;
	std::mt19937 gen(rd());

	// Create distributions with different names
	std::uniform_int_distribution<> xDistrib(0, m_windowConfig.uW);
	std::uniform_int_distribution<> yDistrib(0, m_windowConfig.uH);
	std::uniform_real_distribution<float> speedDistrib(-m_enemyConfig.SMAX, m_enemyConfig.SMAX);
	std::uniform_int_distribution<> verticeDistrib(m_enemyConfig.VMIN, m_enemyConfig.VMAX);
	std::uniform_int_distribution<> colorDistrib(0, 255);

	auto enemy = m_entities.addEntity("enemy");

	enemy->add<CTransform>(Vec2f(xDistrib(gen), yDistrib(gen)),
		Vec2f(speedDistrib(gen), speedDistrib(gen)), 0.0f);
	enemy->add<CShape>(m_enemyConfig.SR, verticeDistrib(gen),
		sf::Color(colorDistrib(gen), colorDistrib(gen), colorDistrib(gen)),
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
		m_enemyConfig.OT);
	enemy->add<CCollision>(m_enemyConfig.CR);

	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{

}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& target)
{
	auto bullet = m_entities.addEntity("bullet");
	auto& entityTransform = entity->get<CTransform>();

	Vec2f bulletVelocity = target - entityTransform.pos;
	bulletVelocity = (bulletVelocity / bulletVelocity.length()) * m_bulletConfig.S;

	bullet->add<CTransform>(entityTransform.pos, bulletVelocity, entityTransform.angle);
	bullet->add<CShape>(m_bulletConfig.SR, m_bulletConfig.V,
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
		m_bulletConfig.OT);
	bullet->add<CCollision>(m_bulletConfig.CR);
	bullet->add<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	
}

void Game::sMovement()
{
	auto& playerTransform = player()->get<CTransform>();
	auto& playerInput = player()->get<CInput>();

	playerTransform.velocity = { 0.0, 0.0 };
	if (playerInput.up) playerTransform.velocity.y -= m_playerConfig.S;
	if (playerInput.left) playerTransform.velocity.x -= m_playerConfig.S;
	if (playerInput.down) playerTransform.velocity.y += m_playerConfig.S;
	if (playerInput.right) playerTransform.velocity.x += m_playerConfig.S;

	if (playerTransform.velocity.x != 0 && playerTransform.velocity.y != 0) {
		float length = playerTransform.velocity.length();
		playerTransform.velocity = (playerTransform.velocity / length) * m_playerConfig.S;
	}

	for (auto& entity : m_entities.getEntities())
	{
		auto& entityTransform = entity->get<CTransform>();
		entityTransform.pos += entityTransform.velocity;
	}
}

void Game::sLifespan()
{
	for (auto& entity : m_entities.getEntities())
	{
		if (!entity->has<CLifespan>()) continue;

		auto& entityLifespan = entity->get<CLifespan>();
		auto& entityShape = entity->get<CShape>();

		sf::Color curFillColor = entityShape.circle.getFillColor();
		sf::Color curOutlineColor = entityShape.circle.getOutlineColor();

		float lifespanRatio = (float)entityLifespan.remaining / (float)entityLifespan.lifespan;
		curFillColor.a = lifespanRatio * 255;
		curOutlineColor.a = lifespanRatio * 255;

		entityShape.circle.setFillColor(curFillColor);
		entityShape.circle.setOutlineColor(curOutlineColor);

		entityLifespan.remaining--;
		if (entityLifespan.remaining <= 0)
		{
			entity->destroy();
		}
	}
}

void Game::sCollision()
{
	auto playerTransform = player()->get<CTransform>();
	auto playerCollision = player()->get<CCollision>();
	for (auto& enemy : m_entities.getEntities("enemy"))
	{
		auto enemyTransform = enemy->get<CTransform>();
		auto enemyCollision = enemy->get<CCollision>();

		for (auto& bullet : m_entities.getEntities("bullet"))
		{
			auto bulletTransform = bullet->get<CTransform>();
			auto bulletCollision = bullet->get<CCollision>();

			if (enemyTransform.pos.distToSquared(bulletTransform.pos)
				< (enemyCollision.radius + bulletCollision.radius)
				* (enemyCollision.radius + bulletCollision.radius))
			{
				enemy->destroy();
				bullet->destroy();
			}
		}

		if (enemyTransform.pos.distToSquared(playerTransform.pos)
			< (enemyCollision.radius + playerCollision.radius)
			* (enemyCollision.radius + playerCollision.radius))
		{
			enemy->destroy();
			// player()->destroy();
		}
	}
}

void Game::sEnemySpawner()
{
	if (m_currentFrame - m_lastEnemySpawnTime > m_enemyConfig.SI)
	{
		spawnEnemy();
	}
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

	for (auto& entity : m_entities.getEntities())
	{
		auto& entityTransform = entity->get<CTransform>();
		auto& entityShape = entity->get<CShape>();

		entityShape.circle.setPosition(entityTransform.pos);
		entityTransform.angle += 1.0f;
		entityShape.circle.setRotation(sf::degrees(entityTransform.angle));

		m_window.draw(entityShape.circle);
	}

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
		
		auto& playerInput = player()->get<CInput>();
		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
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

			switch (mousePressed->button)
			{
			case sf::Mouse::Button::Left:
				playerInput.shoot = true;
				spawnBullet(player(),
					Vec2f(mousePressed->position.x, mousePressed->position.y));
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