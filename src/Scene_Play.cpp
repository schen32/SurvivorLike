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

	m_playerConfig = { 0, 0, 0, 0, 5.0f, 0, ""};

	m_gridText.setCharacterSize(60);
	m_gridText.setFont(m_game->assets().getFont("FutureMillennium"));
	m_gridText.setOutlineThickness(5.0f);
	m_gridText.setOutlineColor(sf::Color(86, 106, 137));

	m_particleSystem.init(m_game->window().getSize());

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

void Scene_Play::loadLevel(const std::string& filename)
{
	m_entityManager = EntityManager();
	spawnPlayer();
	m_entityManager.update();

	std::ifstream file(m_levelPath);
	std::string tileType;
	while (file >> tileType)
	{
		std::string aniName, gridXstr, gridYstr;
		file >> aniName >> gridXstr >> gridYstr;
		float gridX = std::stof(gridXstr);
		float gridY = std::stof(gridYstr);

		if (tileType == "Tile")
		{
			auto tile = m_entityManager.addEntity("Tile");
			auto& eAnimation = tile->add<CAnimation>(m_game->assets().getAnimation(aniName), true);
			tile->add<CTransform>(gridToMidPixel(gridX, gridY, tile), Vec2f(0, 0), 0);
			tile->add<CBoundingBox>(eAnimation.animation.m_size);
		}
	}
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
	p->add<CTransform>(Vec2f(m_playerConfig.X, m_playerConfig.Y));
	auto& eAnimation = p->add<CAnimation>(m_game->assets().getAnimation("PlayerIdle"), true);
	p->add<CInput>();
	p->add<CBoundingBox>(Vec2f(eAnimation.animation.m_size.x, eAnimation.animation.m_size.y));

}

void Scene_Play::update()
{
	m_entityManager.update();
	if (!m_paused)
	{
		sMovement();
		sCollision();
	}
	sAnimation();
}

void Scene_Play::sScore()
{
	
}

void Scene_Play::sDrag()
{
	for (auto& entity : m_entityManager.getEntities())
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
	}
}

void Scene_Play::sDespawn()
{
	
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

}

void Scene_Play::sStatus()
{

}

void Scene_Play::sCollision()
{
	for (auto& tile : m_entityManager.getEntities("Tile"))
	{
		Vec2f overlap = Physics::GetOverlap(player(), tile);
		if (overlap.x > 0 && overlap.y > 0)
		{
			Vec2f prevOverlap = Physics::GetPreviousOverlap(player(), tile);
			auto& pTransform = player()->get<CTransform>();
			auto& tileTransform = tile->get<CTransform>();

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

	m_gridText.setString("Hello World");
	window.draw(m_gridText);

	/*m_particleSystem.update();
	m_particleSystem.draw(window);*/
}