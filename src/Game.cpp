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

	m_font.openFromFile("font/CaviarDreams.ttf");
	m_text.setCharacterSize(30);
	m_text.setFillColor(sf::Color::White);
	m_text.setPosition({ 30.0f, 20.0f });

	// int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S
	m_playerConfig = { 32, 32, 100, 100, 100, 250, 250, 250, 4, 6, 5.0f };
	// int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S
	m_bulletConfig = { 6, 6, 250, 250, 250, 250, 250, 250, 2, 12, 100, 15.0f };
	// int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX
	m_enemyConfig = { 40, 40, 250, 10, 10, 3, 3, 7, 60, 120, 2.0f, 6.0f };

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

		if (!m_paused)
		{
			sEnemySpawner();
			sMovement();
			sLifespan();
			sCollision();
		}
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
	std::uniform_int_distribution<> xDistrib(m_enemyConfig.SR,
		m_windowConfig.uW - m_enemyConfig.SR);
	std::uniform_int_distribution<> yDistrib(m_enemyConfig.SR,
		m_windowConfig.uH - m_enemyConfig.SR);
	std::uniform_real_distribution<float> speedDistrib(-m_enemyConfig.SMAX, m_enemyConfig.SMAX);
	std::uniform_int_distribution<> verticeDistrib(m_enemyConfig.VMIN, m_enemyConfig.VMAX);
	std::uniform_int_distribution<> colorDistrib(0, 255);
	int vertices = verticeDistrib(gen);

	auto enemy = m_entities.addEntity("enemy");

	enemy->add<CTransform>(Vec2f(xDistrib(gen), yDistrib(gen)),
		Vec2f(speedDistrib(gen), speedDistrib(gen)), 0.0f);
	enemy->add<CShape>(m_enemyConfig.SR, vertices,
		sf::Color(colorDistrib(gen), colorDistrib(gen), colorDistrib(gen)),
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
		m_enemyConfig.OT);
	enemy->add<CCollision>(m_enemyConfig.CR);
	enemy->add<CScore>(vertices);

	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{
	auto& parentTransform = entity->get<CTransform>();
	auto& parentShape = entity->get<CShape>();

	size_t vertices = parentShape.circle.getPointCount();
	for (size_t i = 0; i < vertices; i++)
	{
		auto enemySmall = m_entities.addEntity("enemySmall");

		float angle = i * (2 * 3.14159 / vertices);
		Vec2f angleVec = Vec2f(std::cos(angle), std::sin(angle));
		Vec2f velocity = angleVec * parentTransform.velocity.length();

		enemySmall->add<CTransform>(parentTransform.pos + velocity,
			velocity, 0.0f);
		enemySmall->add<CShape>(m_enemyConfig.SR / 2, vertices,
			parentShape.circle.getFillColor(),
			parentShape.circle.getOutlineColor(),
			m_enemyConfig.OT);
		enemySmall->add<CCollision>(m_enemyConfig.CR / 2);
		enemySmall->add<CLifespan>(m_enemyConfig.L);
		enemySmall->add<CScore>(vertices);
	}

	entity->destroy();
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

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2f& target)
{
	auto& entityTransform = entity->get<CTransform>();
	Vec2f diff = target - entityTransform.pos;
	float angle = std::atan2f(diff.y, diff.x);

	Vec2f centerDir(std::cos(angle), std::sin(angle));
	spawnBullet(entity, entityTransform.pos + centerDir);

	const float SPREAD_ANGLE = 5.0f * (2 * 3.14159 / 360.0f);
	Vec2f leftDir(std::cos(angle - SPREAD_ANGLE), std::sin(angle - SPREAD_ANGLE));
	spawnBullet(entity, entityTransform.pos + leftDir);

	Vec2f rightDir(std::cos(angle + SPREAD_ANGLE), std::sin(angle + SPREAD_ANGLE));
	spawnBullet(entity, entityTransform.pos + rightDir);
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
	for (auto& bullet : m_entities.getEntities("bullet"))
	{
		auto& bulletTransform = bullet->get<CTransform>();
		auto& bulletCollision = bullet->get<CCollision>();

		for (auto& enemy : m_entities.getEntities("enemy"))
		{
			auto& enemyTransform = enemy->get<CTransform>();
			auto& enemyCollision = enemy->get<CCollision>();

			if (enemyTransform.pos.distToSquared(bulletTransform.pos)
				< (enemyCollision.radius + bulletCollision.radius)
				* (enemyCollision.radius + bulletCollision.radius))
			{
				m_score += enemy->get<CScore>().score;
				spawnSmallEnemies(enemy);
				bullet->destroy();
			}
		}

		for (auto& enemySmall : m_entities.getEntities("enemySmall"))
		{
			auto& enemyTransform = enemySmall->get<CTransform>();
			auto& enemyCollision = enemySmall->get<CCollision>();

			if (enemyTransform.pos.distToSquared(bulletTransform.pos)
				< (enemyCollision.radius + bulletCollision.radius)
				* (enemyCollision.radius + bulletCollision.radius))
			{
				m_score += enemySmall->get<CScore>().score;
				enemySmall->destroy();
				bullet->destroy();
			}
		}
	}
	auto& playerTransform = player()->get<CTransform>();
	auto& playerShape = player()->get<CShape>();
	auto& playerCollision = player()->get<CCollision>();
	for (auto& enemy : m_entities.getEntities("enemy"))
	{
		auto& enemyTransform = enemy->get<CTransform>();
		auto& enemyCollision = enemy->get<CCollision>();

		if (enemyTransform.pos.distToSquared(playerTransform.pos)
			< (enemyCollision.radius + playerCollision.radius)
			* (enemyCollision.radius + playerCollision.radius))
		{
			spawnSmallEnemies(enemy);
			playerShape.circle.setPointCount(playerShape.circle.getPointCount() - 1);
		}

		for (auto& otherEnemy : m_entities.getEntities("enemy"))
		{
			if (enemy->id() == otherEnemy->id()) continue;

			auto& otherEnemyTransform = otherEnemy->get<CTransform>();
			auto& otherEnemyCollision = otherEnemy->get<CCollision>();

			if (enemyTransform.pos.distToSquared(otherEnemyTransform.pos)
				< (enemyCollision.radius + otherEnemyCollision.radius)
				* (enemyCollision.radius + otherEnemyCollision.radius))
			{
				spawnSmallEnemies(otherEnemy);
			}
		}
	}

	for (auto& enemy : m_entities.getEntities("enemySmall"))
	{
		auto& enemyTransform = enemy->get<CTransform>();
		auto& enemyCollision = enemy->get<CCollision>();

		if (enemyTransform.pos.distToSquared(playerTransform.pos)
			< (enemyCollision.radius + playerCollision.radius)
			* (enemyCollision.radius + playerCollision.radius))
		{
			enemy->destroy();
			playerShape.circle.setPointCount(playerShape.circle.getPointCount() - 1);
		}
	}

	// entities bounce off walls
	for (auto& entity : m_entities.getEntities())
	{
		auto& entityTransform = entity->get<CTransform>();
		auto& entityCollision = entity->get<CCollision>();
		if (entityTransform.pos.x <= entityCollision.radius ||
			entityTransform.pos.x >= m_windowConfig.fW - entityCollision.radius)
		{
			entityTransform.velocity.x *= -1;
		}
		if (entityTransform.pos.y <= entityCollision.radius ||
			entityTransform.pos.y >= m_windowConfig.fH - entityCollision.radius)
		{
			entityTransform.velocity.y *= -1;
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
	m_text.setString("Score: " + std::to_string(m_score));
	m_window.draw(m_text);

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
				setPaused(!m_paused);
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
				spawnBullet(player(),
					Vec2f(mousePressed->position.x, mousePressed->position.y));
				break;
			case sf::Mouse::Button::Right:
				spawnSpecialWeapon(player(),
					Vec2f(mousePressed->position.x, mousePressed->position.y));
				break;
			default: break;
			}
		}
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}