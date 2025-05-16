#pragma once

#include "imgui-SFML.h"
#include "imgui.h"
#include "EntityManager.hpp"

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
struct WindowConfig { unsigned int FR, uW, uH; float fW, fH; };

class Game
{
	sf::RenderWindow m_window;
	WindowConfig m_windowConfig = { 60, 1600, 800, 1600.0f, 800.0f };

	EntityManager m_entities;
	sf::Font m_font;
	sf::Text m_text = sf::Text(m_font);
	PlayerConfig m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConfig m_bulletConfig;
	sf::Clock m_deltaClock;
	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	bool m_paused = false;
	bool m_running = true;

	void init(const std::string& config);
	void setPaused(bool paused);

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sGUI();
	void sEnemySpawner();
	void sCollision();

	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);

	std::shared_ptr<Entity> player();

public:
	Game(const std::string& config);

	void run();

};