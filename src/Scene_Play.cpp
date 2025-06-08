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
	registerAction(sf::Keyboard::Scan::Space, "TOGGLE_AUTO_ATTACK");

	m_playerConfig = { 0, 0, 0, 0, 4.0f, 0, ""};

	m_gridText.setCharacterSize(100);
	m_gridText.setFont(m_game->assets().getFont("FutureMillennium"));
	m_gridText.setOutlineThickness(3.0f);
	m_gridText.setOutlineColor(sf::Color(86, 106, 137));
	m_gridText.setPosition({ 20, 10 });

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

std::shared_ptr<Entity> Scene_Play::getNearestEnemy(std::shared_ptr<Entity> entity)
{
	float minDist = 1000;
	std::shared_ptr<Entity> nearestEnemy = nullptr;

	auto& eTransform = entity->get<CTransform>();
	for (auto& enemy : m_entityManager.getEntities("enemy"))
	{
		if (enemy->id() == entity->id())
			continue;

		auto& enemyTransform = enemy->get<CTransform>();
		float dist = (enemyTransform.pos - eTransform.pos).length();
		if (dist < minDist)
		{
			minDist = dist;
			nearestEnemy = enemy;
		}
	}
	return nearestEnemy;
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
	p->add<CHealth>(5);
	p->add<CInput>();
	p->add<CScore>();

	p->add<CBasicAttack>(m_currentFrame);
	p->add<CSpecialAttack>(m_currentFrame);
}

void Scene_Play::spawnEnemies()
{
	static int lastEnemySpawnTime = 0;
	static const int enemySpawnInterval = 30;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * 500;

		auto enemy = m_entityManager.addEntity("enemy");
		auto& eAnimation = enemy->add<CAnimation>(m_game->assets().getAnimation("ChainBotIdle"), true);
		enemy->add<CTransform>(player()->get<CTransform>().pos + spawnPoint);
		enemy->add<CBoundingBox>(eAnimation.animation.m_size / 4);
		enemy->add<CHealth>(2);
		enemy->add<CFollow>(player());
		enemy->add<CScore>(1);
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
		sLifespan();
		sPlayerAttacks();
		spawnEnemies();
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

	pTransform.velocity = { 0.f, 0.f };

	if (pInput.left)  pTransform.velocity.x -= 1.f;
	if (pInput.right) pTransform.velocity.x += 1.f;
	if (pInput.up)    pTransform.velocity.y -= 1.f;
	if (pInput.down)  pTransform.velocity.y += 1.f;

	// Normalize if necessary
	if (pTransform.velocity.x != 0.f || pTransform.velocity.y != 0.f)
	{
		pTransform.velocity = pTransform.velocity.normalize() * m_playerConfig.SPEED;
	}


	for (auto& entity : m_entityManager.getEntities())
	{
		auto& eTransform = entity->get<CTransform>();

		if (entity->has<CMoveAtSameVelocity>())
		{
			auto& target = entity->get<CMoveAtSameVelocity>().target;
			auto& tTransform = target->get<CTransform>();
			eTransform.velocity = tTransform.velocity;
		}

		eTransform.prevPos = eTransform.pos;
		eTransform.pos += eTransform.velocity;
	}
}

void Scene_Play::sAI()
{
	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CFollow>())
			continue;
		auto& eFollow = entity->get<CFollow>();
		auto& tTransform = eFollow.target->get<CTransform>();
		auto& eTransform = entity->get<CTransform>();

		Vec2f desired = (tTransform.pos - eTransform.pos).normalize();
		Vec2f steering = (desired - eTransform.velocity) * eFollow.steering_scale;
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
					{
						e1->destroy();
						player()->get<CScore>().score += e1->get<CScore>().score;
					}
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
	if (pInput.autoAttack)
	{
		auto nearestEnemy = getNearestEnemy(player());
		if (!nearestEnemy)
			return;
		auto& neTransform = nearestEnemy->get<CTransform>();

		spawnBasicAttack(neTransform.pos);
		spawnSpecialAttack(neTransform.pos);
		return;
	}
	
	if (pInput.basicAttack)
		spawnBasicAttack(m_mousePos);
	if (pInput.specialAttack)
		spawnSpecialAttack(m_mousePos);
}

void Scene_Play::spawnBasicAttack(const Vec2f& targetPos)
{
	if (!player()->has<CBasicAttack>())
		return;

	auto& pBasicAttack = player()->get<CBasicAttack>();
	if ((m_currentFrame - pBasicAttack.lastAttackTime) < pBasicAttack.cooldown)
		return;
	pBasicAttack.lastAttackTime = m_currentFrame;

	auto& pTransform = player()->get<CTransform>();

	Vec2f attackDir = (targetPos - pTransform.pos).normalize();
	auto basicAttack = m_entityManager.addEntity("playerAttack");

	float attackAngle = std::atan2(attackDir.y, attackDir.x) * 180.0f / 3.14159f;
	basicAttack->add<CTransform>(pTransform.pos + attackDir * pBasicAttack.distanceFromPlayer
		, Vec2f(0, 0), attackAngle + 225);

	auto& baAnimation = basicAttack->add<CAnimation>(m_game->assets().getAnimation("BasicAttack"), true).animation;
	baAnimation.m_sprite.setScale({ pBasicAttack.scale, pBasicAttack.scale });

	basicAttack->add<CBoundingBox>(baAnimation.m_size * pBasicAttack.scale);
	basicAttack->add<CLifespan>(pBasicAttack.duration, m_currentFrame);
	basicAttack->add<CHealth>(pBasicAttack.pierce);
	basicAttack->add<CMoveAtSameVelocity>(player());
}

void Scene_Play::spawnSpecialAttack(const Vec2f& targetPos)
{
	if (!player()->has<CSpecialAttack>())
		return;

	auto& pSpecialAttack = player()->get<CSpecialAttack>();
	if ((m_currentFrame - pSpecialAttack.lastAttackTime) < pSpecialAttack.cooldown)
		return;
	pSpecialAttack.lastAttackTime = m_currentFrame;

	auto& pTransform = player()->get<CTransform>();

	Vec2f attackDir = (targetPos - pTransform.pos).normalize();
	auto basicAttack = m_entityManager.addEntity("playerAttack");

	float attackAngle = std::atan2(attackDir.y, attackDir.x) * 180.0f / 3.14159f;
	basicAttack->add<CTransform>(pTransform.pos + attackDir, attackDir * pSpecialAttack.speed, attackAngle + 225);

	auto& baAnimation = basicAttack->add<CAnimation>(m_game->assets().getAnimation("BasicAttack"), true).animation;
	baAnimation.m_sprite.setScale({ pSpecialAttack.scale, pSpecialAttack.scale });

	basicAttack->add<CBoundingBox>(baAnimation.m_size * pSpecialAttack.scale);
	basicAttack->add<CLifespan>(pSpecialAttack.duration, m_currentFrame);
	basicAttack->add<CHealth>(pSpecialAttack.pierce);
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
		else if (action.m_name == "RIGHT_CLICK")
		{
			pInput.specialAttack = true;
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		}
		else if (action.m_name == "MOUSE_MOVE")
		{
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		}
		else if (action.m_name == "TOGGLE_AUTO_ATTACK")
		{
			pInput.autoAttack = !pInput.autoAttack;
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
		else if (action.m_name == "RIGHT_CLICK")
		{
			pInput.specialAttack = false;
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

	m_gridText.setString(std::to_string(player()->get<CScore>().score));
	window.draw(m_gridText);

	/*m_particleSystem.update();
	m_particleSystem.draw(window);*/

	window.setView(m_cameraView);
}