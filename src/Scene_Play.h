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
	Vec2f					 m_mousePos = { 0, 0 };
	ParticleSystem			 m_particleSystem;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);

	void onEnd();
	void update();
	void spawnPlayer();
	std::shared_ptr<Entity> player();
	void sDoAction(const Action& action);
	Vec2f gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);

	void sScore();
	void sDrag();
	void sDespawn();
	void sMovement();
	void sAI();
	void sStatus();
	void sAnimation();
	void sCollision();
	void sCamera();
	void sGui();

public:

	Scene_Play() = default;
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath);

	void sRender();
};
