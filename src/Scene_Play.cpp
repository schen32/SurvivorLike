#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "ParticleSystem.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <math.h>

bool IsInside(const Vec2f& pos, std::shared_ptr<Entity> entity)
{
	auto& ePosition = entity->get<CTransform>().pos;
	auto& eSize = entity->get<CAnimation>().animation.m_size;

	if (ePosition.x - eSize.x / 2 <= pos.x && pos.x <= ePosition.x + eSize.x / 2 &&
		ePosition.y - eSize.y / 2 <= pos.y && pos.y <= ePosition.y + eSize.y / 2)
	{
		return true;
	}
	return false;
}

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
	registerAction(sf::Keyboard::Scan::P, "PAUSE");
	registerAction(sf::Keyboard::Scan::Escape, "QUIT");
	registerAction(sf::Keyboard::Scan::H, "DISPLAY_HITBOX");

	registerAction(sf::Keyboard::Scan::A, "LEFT");
	registerAction(sf::Keyboard::Scan::D, "RIGHT");
	registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");

	m_playerConfig = { 0, 0, 0, 0, 4.0f, 0, ""};

	m_gridText.setCharacterSize(30);
	m_gridText.setFont(m_game->assets().getFont("FutureMillennium"));
	m_gridText.setOutlineThickness(3.0f);
	m_gridText.setOutlineColor(sf::Color(86, 106, 137));

	m_particleSystem.init(m_game->window().getSize());
	m_cameraView.setSize({ (float)width(), (float)height() });
	m_cameraView.zoom(0.5f);

	loadLevel(levelPath);
}

Vec2f Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	auto& eAnimation = entity->get<CAnimation>();
	Vec2f eAniSize = eAnimation.animation.m_size;
	
	return Vec2f
	(
		gridX * eAniSize.x + eAniSize.x / 2,
		height() - gridY * eAniSize.y - eAniSize.y / 2
	);
}

void Scene_Play::loadLevel(const std::string& filename = "")
{
	m_entityManager = EntityManager();
	spawnPlayer();
	spawnTiles(filename);
	m_entityManager.update();
}

std::shared_ptr<Entity> Scene_Play::player()
{
	auto& player = m_entityManager.getEntities("player");
	assert(player.size() == 1);
	return player.front();
}

void Scene_Play::spawnPlayer()
{
	auto p = m_entityManager.addEntity("player");
	
	auto& pAnimation = p->add<CAnimation>(m_game->assets().getAnimation("StormheadIdle"), true);
	p->add<CBoundingBox>(Vec2f(pAnimation.animation.m_size.x / 4, pAnimation.animation.m_size.y / 4));
	p->add<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, p));
	p->add<CBasicAttack>(30, m_currentFrame);
	p->add<CHealth>(5);
	p->add<CInput>();
}

void Scene_Play::spawnEnemies()
{
	static int lastEnemySpawnTime = 0;
	static const int enemySpawnInterval = 60;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * 500;
		auto& pPos = player()->get<CTransform>().pos;

		auto enemy = m_entityManager.addEntity("enemy");
		auto& eAnimation = enemy->add<CAnimation>(m_game->assets().getAnimation("ChainBotIdle"), true);
		enemy->add<CTransform>(pPos + spawnPoint);
		enemy->add<CBoundingBox>(eAnimation.animation.m_size / 4);
		enemy->add<CHealth>(2);

	}
}

void Scene_Play::spawnTiles(const std::string& filename)
{
	
}

void Scene_Play::update()
{
	m_entityManager.update();
	if (!m_paused)
	{
		spawnEnemies();
		sLifespan();
		sPlayerAttacks();
		sAI();
		sMovement();
		sCollision();
	}
	sAnimation();
	sCamera();
}

void Scene_Play::sScore()
{

}

void Scene_Play::sDrag()
{
	/*for (auto& entity : m_entityManager.getEntities())
	{
		if (entity->has<CDraggable>() && entity->get<CDraggable>().dragging)
		{
			auto& eTransform = entity->get<CTransform>();
			eTransform.pos = m_mousePos;
			if (entity->id() == player()->id())
			{
				eTransform.velocity = { 0, 0 };
			}
		}
	}*/
}

void Scene_Play::sMovement()
{
	auto& pInput = player()->get<CInput>();
	auto& pTransform = player()->get<CTransform>();

	pTransform.velocity = { 0, 0 };
	if (pInput.left)
		pTransform.velocity.x -= m_playerConfig.SPEED;
	if (pInput.right)
		pTransform.velocity.x += m_playerConfig.SPEED;
	if (pInput.up)
		pTransform.velocity.y -= m_playerConfig.SPEED;
	if (pInput.down)
		pTransform.velocity.y += m_playerConfig.SPEED;

	for (auto& entity : m_entityManager.getEntities())
	{
		auto& eTransform = entity->get<CTransform>();

		eTransform.prevPos = eTransform.pos;
		eTransform.pos += eTransform.velocity;
	}
}

void Scene_Play::sAI()
{
	auto& pTransform = player()->get<CTransform>();
	for (auto& entity : m_entityManager.getEntities("enemy"))
	{
		const static float STEERING_SCALE = 0.1f;

		auto& eTransform = entity->get<CTransform>();
		Vec2f desired = (pTransform.pos - eTransform.pos).normalize();
		Vec2f steering = (desired - eTransform.velocity) * STEERING_SCALE;
		eTransform.velocity += steering;
	}
}

void Scene_Play::sStatus()
{

}

void Scene_Play::sCollision()
{
	for (auto& e1 : m_entityManager.getEntities("enemy"))
	{
		if (!e1->has<CBoundingBox>())
			continue;

		for (auto& e2 : m_entityManager.getEntities())
		{
			if (!e2->has<CBoundingBox>() || e1->id() == e2->id())
				continue;

			Vec2f overlap = Physics::GetOverlap(e1, e2);
			if (overlap.x > 0 && overlap.y > 0)
			{
				if (e2->tag() != "enemy" && e2->has<CHealth>())
				{
					auto& e1Health = e1->get<CHealth>().health;
					auto& e2Health = e2->get<CHealth>().health;
					e1Health--;
					e2Health--;

					if (e2->id() == player()->id() && e2Health <= 0)
					{
						loadLevel();
						return;
					}

					if (e1Health <= 0)
						e1->destroy();
					if (e2Health <= 0)
						e2->destroy();
				}

				Vec2f prevOverlap = Physics::GetPreviousOverlap(e1, e2);
				auto& pTransform = e1->get<CTransform>();
				auto& tileTransform = e2->get<CTransform>();

				if (prevOverlap.x > 0)
				{
					pTransform.velocity.y = 0;
					if (pTransform.prevPos.y < tileTransform.pos.y)
						pTransform.pos.y -= overlap.y;
					else
						pTransform.pos.y += overlap.y;
				}
				else if (prevOverlap.y > 0)
				{
					pTransform.velocity.x = 0;
					if (pTransform.prevPos.x < tileTransform.pos.x)
						pTransform.pos.x -= overlap.x;
					else
						pTransform.pos.x += overlap.x;
				}
			}
		}
	}
}

void Scene_Play::sLifespan()
{
	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CLifespan>())
			continue;

		auto& eLifespan = entity->get<CLifespan>();
		if (m_currentFrame - eLifespan.frameCreated > eLifespan.lifespan)
		{
			entity->destroy();
		}
	}
}

void Scene_Play::sPlayerAttacks()
{
	auto& pInput = player()->get<CInput>();
	if (pInput.basicAttack && player()->has<CBasicAttack>())
	{
		auto& pBA = player()->get<CBasicAttack>();
		if ((m_currentFrame - pBA.lastAttackTime) > pBA.cooldown)
		{
			pBA.lastAttackTime = m_currentFrame;

			auto basicAttack = m_entityManager.addEntity("playerAttack");

			auto& pTransform = player()->get<CTransform>();
			Vec2f attackDir = (m_mousePos - pTransform.pos).normalize() * 30;

			float attackAngle = std::atan2(attackDir.y, attackDir.x) * 180.0f / 3.14159f;
			basicAttack->add<CTransform>(pTransform.pos + attackDir, Vec2f(0, 0), attackAngle + 225);

			auto& baAnimation = basicAttack->add<CAnimation>(m_game->assets().getAnimation("BasicAttack"), true).animation;
			baAnimation.m_sprite.setScale({ pBA.scale, pBA.scale });

			basicAttack->add<CBoundingBox>(baAnimation.m_size * pBA.scale);
			basicAttack->add<CLifespan>(pBA.duration, m_currentFrame);
			basicAttack->add<CHealth>(10);
		}
	}
}

void Scene_Play::sDoAction(const Action& action)
{
	auto& pInput = player()->get<CInput>();
	if (action.m_type == "START")
	{
		if (action.m_name == "LEFT")
		{
			pInput.left = true;
		}
		else if (action.m_name == "RIGHT")
		{
			pInput.right = true;
		}
		else if (action.m_name == "UP")
		{
			pInput.up = true;
		}
		else if (action.m_name == "DOWN")
		{
			pInput.down = true;
		}
		else if (action.m_name == "QUIT")
		{
			onEnd();
		}
		else if (action.m_name == "PAUSE")
		{
			m_paused = !m_paused;
		}
		else if (action.m_name == "DISPLAY_HITBOX")
		{
			pInput.displayHitbox = !pInput.displayHitbox;
		}
		else if (action.m_name == "LEFT_CLICK")
		{
			pInput.basicAttack = true;
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		}
		else if (action.m_name == "MOUSE_MOVE")
		{
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		}
	}
	else if (action.m_type == "END")
	{
		if (action.m_name == "LEFT")
		{
			pInput.left = false;
		}
		else if (action.m_name == "RIGHT")
		{
			pInput.right = false;
		}
		if (action.m_name == "UP")
		{
			pInput.up = false;
		}
		else if (action.m_name == "DOWN")
		{
			pInput.down = false;
		}
		else if (action.m_name == "LEFT_CLICK")
		{
			pInput.basicAttack = false;
		}
	}
}

void Scene_Play::sAnimation()
{
	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CAnimation>())
			continue;

		auto& eAnimation = entity->get<CAnimation>();
		eAnimation.animation.update();

		if (!eAnimation.repeat && eAnimation.animation.hasEnded())
			entity->destroy();
	}
}

void Scene_Play::sCamera()
{
	auto& pTransform = player()->get<CTransform>();
	m_cameraView.setCenter(pTransform.pos);
	m_game->window().setView(m_cameraView);
}

void Scene_Play::onEnd()
{
	m_game->quit();
}

void Scene_Play::sGui()
{

}

void Scene_Play::sRender()
{
	auto& window = m_game->window();
	window.clear(sf::Color(204, 226, 225));

	for (auto& entity : m_entityManager.getEntities())
	{
		auto& transform = entity->get<CTransform>();
		auto& animation = entity->get<CAnimation>().animation;

		animation.m_sprite.setPosition(transform.pos);
		animation.m_sprite.setRotation(sf::degrees(transform.angle));
		window.draw(animation.m_sprite);

		if (player()->get<CInput>().displayHitbox)
		{
			auto& boundingBox = entity->get<CBoundingBox>();
			sf::RectangleShape hitbox(boundingBox.size);
			hitbox.setOrigin(boundingBox.halfSize);
			hitbox.setPosition(transform.pos);
			hitbox.setFillColor(sf::Color(255, 0, 0, 50));
			hitbox.setOutlineColor(sf::Color::Red);
			hitbox.setOutlineThickness(1.f);
			window.draw(hitbox);
		}
	}
	window.setView(window.getDefaultView());

	m_gridText.setString("Hello World");
	window.draw(m_gridText);

	/*m_particleSystem.update();
	m_particleSystem.draw(window);*/

	window.setView(m_cameraView);
}