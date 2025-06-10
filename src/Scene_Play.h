#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.hpp"
#include "ParticleSystem.hpp"

class Scene_Play : public Scene
{
	struct PlayerConfig
	{
		float X = 0, Y = 0, CX = 0, CY = 0, SPEED = 0, HEALTH = 0;
		std::string WEAPON;
	};

protected:

	std::shared_ptr<Entity>  m_block;
	std::string              m_levelPath;
	std::string				 m_lastAction;
	PlayerConfig             m_playerConfig;
	bool                     m_drawTextures = true;
	bool                     m_drawCollision = false;
	bool                     m_drawGrid = false;
	const Vec2f              m_gridSize = { 64, 64 };
	sf::Font				 m_fontText;
	sf::Text                 m_gridText = sf::Text(m_fontText);
	ParticleSystem			 m_particleSystem;
	sf::View				 m_cameraView;
	Vec2f					 m_mousePos;
	bool					 m_playerDied = false;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);

	void onEnd();
	void update();
	void spawnPlayer();

	void sSpawnEnemies();
	void spawnChainBot();
	void spawnBigChainBot();
	void spawnBotWheel();
	void spawnBigBotWheel();
	void enemyDied(std::shared_ptr<Entity> enemy);

	void spawnGem(const Vec2f& pos);
	void spawnTiles(const std::string& filename);
	std::shared_ptr<Entity> player();
	void sDoAction(const Action& action);
	Vec2f gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
	std::shared_ptr<Entity> getNearestEnemy(std::shared_ptr<Entity> entity);

	void sScore();
	void sDrag();
	void sLifespan();
	void sAttraction();
	void sKnockback();
	void sMovement();
	void sAI();
	void sStatus();
	void sAnimation();
	void sSound();
	void sCollision();
	void sDisappearingText();
	void sCamera();
	void sGui();

	void sPlayerAttacks();
	void spawnBasicAttack(const Vec2f& targetPos);
	void spawnSpecialAttack(const Vec2f& targetPos);
	void spawnRingAttack(const Vec2f& targetPos);
	void spawnExplodeAttack(const Vec2f& targetPos);
	void spawnWhirlAttack(const Vec2f& targetPos);
	void applyKnockback(std::shared_ptr<Entity> target, const Vec2f& fromPos, float force, int duration);
	bool applyAttraction(std::shared_ptr<Entity> attractor, std::shared_ptr<Entity> target);
	void spawnDisappearingText(const std::string& text, const Vec2f& pos);
	void playSound(const std::string& name, float volume);
	bool applyDamage(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2);

	void renderShadow(std::shared_ptr<Entity> entity);
public:

	Scene_Play() = default;
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath);

	void sRender();
};
