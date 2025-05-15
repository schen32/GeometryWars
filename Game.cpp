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