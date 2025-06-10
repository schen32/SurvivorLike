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

	m_playerConfig = { 0, 0, 0, 0, 3.0f, 0, ""};

	m_gridText.setCharacterSize(100);
	m_gridText.setFont(m_game->assets().getFont("FutureMillennium"));
	m_gridText.setOutlineThickness(3.0f);
	m_gridText.setOutlineColor(sf::Color(86, 106, 137));
	m_gridText.setPosition({ 20, 10 });

	m_particleSystem.init(m_game->window().getSize());
	m_cameraView.setSize({ (float)width(), (float)height() });
	m_cameraView.zoom(0.5f);

	srand(static_cast<unsigned int>(time(nullptr)));
	std::vector<std::string> bgms = { "EveningHarmony", "FloatingDream", "ForgottenBiomes",
		"PolarLights", "SunlightThroughLeaves" };
	int randomIndex = rand() % bgms.size();

	auto& bgm = m_game->assets().getMusic(bgms[randomIndex]);
	bgm.setVolume(120);
	bgm.setLooping(true);
	bgm.play();

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
	auto p = m_entityManager.addEntity("player", "playerCharacter");
	m_playerDied = false;
	
	auto& pAnimation = p->add<CAnimation>(m_game->assets().getAnimation("StormheadIdle"), true);
	p->add<CBoundingBox>(Vec2f(pAnimation.animation.m_size.x / 4, pAnimation.animation.m_size.y / 4));
	p->add<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, p));
	p->add<CHealth>(100);
	p->add<CDamage>(10);
	p->add<CInput>();
	p->add<CScore>(0);
	p->add<CState>("idle");
	p->add<CKnockback>(10.f, 30);

	p->add<CBasicAttack>(m_currentFrame);
	p->add<CSpecialAttack>(m_currentFrame);
}

void Scene_Play::sSpawnEnemies()
{
	spawnChainBot();
	spawnBigChainBot();
	spawnBotWheel();
	spawnBigBotWheel();
}

void Scene_Play::spawnChainBot()
{
	static int lastEnemySpawnTime = 0;
	static const int enemySpawnInterval = 60;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * 500;

		auto enemy = m_entityManager.addEntity("enemy", "chainBot");
		auto& eAnimation = enemy->add<CAnimation>(m_game->assets().getAnimation("ChainBotIdle"), true);
		enemy->add<CTransform>(player()->get<CTransform>().pos + spawnPoint);
		enemy->add<CBoundingBox>(eAnimation.animation.m_size / 2);
		enemy->add<CHealth>(30);
		enemy->add<CDamage>(10);
		enemy->add<CFollow>(player(), 0.5f);
		enemy->add<CScore>(1);
		enemy->add<CState>("alive");
	}
}

void Scene_Play::spawnBotWheel()
{
	static int lastEnemySpawnTime = 0;
	static const int enemySpawnInterval = 90;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * 500;

		auto enemy = m_entityManager.addEntity("enemy", "botWheel");
		auto& eAnimation = enemy->add<CAnimation>(m_game->assets().getAnimation("BotWheelRun"), true);
		enemy->add<CTransform>(player()->get<CTransform>().pos + spawnPoint);
		enemy->add<CBoundingBox>(eAnimation.animation.m_size / 2);
		enemy->add<CHealth>(40);
		enemy->add<CDamage>(10);
		enemy->add<CFollow>(player(), 0.6f);
		enemy->add<CScore>(2);
		enemy->add<CState>("alive");
	}
}

void Scene_Play::spawnBigChainBot()
{
	static int lastEnemySpawnTime = 0;
	static const int enemySpawnInterval = 900;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * 500;

		auto enemy = m_entityManager.addEntity("enemy", "chainBot");
		auto& eTransform = enemy->add<CTransform>(player()->get<CTransform>().pos + spawnPoint);
		eTransform.scale = 2.0f;

		auto& eAnimation = enemy->add<CAnimation>(m_game->assets().getAnimation("ChainBotIdle"), true);
		eAnimation.animation.m_sprite.setScale(Vec2f(eTransform.scale, eTransform.scale));
		
		enemy->add<CBoundingBox>(eAnimation.animation.m_size / 2 * eTransform.scale);
		enemy->add<CHealth>(200);
		enemy->add<CDamage>(20);
		enemy->add<CFollow>(player(), 0.3f);
		enemy->add<CScore>(6);
		enemy->add<CState>("alive");
	}
}

void Scene_Play::spawnBigBotWheel()
{
	static int lastEnemySpawnTime = 0;
	static const int enemySpawnInterval = 1200;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * 500;

		auto enemy = m_entityManager.addEntity("enemy", "botWheel");
		auto& eTransform = enemy->add<CTransform>(player()->get<CTransform>().pos + spawnPoint);
		eTransform.scale = 2.0f;

		auto& eAnimation = enemy->add<CAnimation>(m_game->assets().getAnimation("BotWheelRun"), true);
		eAnimation.animation.m_sprite.setScale(Vec2f(eTransform.scale, eTransform.scale));

		enemy->add<CBoundingBox>(eAnimation.animation.m_size / 2 * eTransform.scale);
		enemy->add<CHealth>(250);
		enemy->add<CDamage>(20);
		enemy->add<CFollow>(player(), 0.4f);
		enemy->add<CScore>(8);
		enemy->add<CState>("alive");
	}
}

void Scene_Play::spawnGem(const Vec2f& pos)
{
	int spawnAngle = rand() % 360;
	Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * 10;

	auto gem = m_entityManager.addEntity("gem", "scoreGem");

	gem->add<CTransform>(pos + spawnPoint);
	auto& gemAnimation = gem->add<CAnimation>(m_game->assets().getAnimation("Gem"), true);
	gem->add<CBoundingBox>(gemAnimation.animation.m_size);
	gem->add<CScore>(1);
	gem->add<CFollow>(player(), 3.0f);
}

void Scene_Play::spawnTiles(const std::string& filename)
{
	
}

void Scene_Play::update()
{
	if (!m_paused)
	{
		m_entityManager.update();
		sLifespan();
		sDisappearingText();
		sSpawnEnemies();
		sPlayerAttacks();
		sKnockback();
		sAI();
		sMovement();
		sCollision();
		sScore();
		sSound();
		sCamera();
		sAnimation();
	}

	if (m_playerDied)
	{
		loadLevel();
	}
}

void Scene_Play::sScore()
{
	auto& pScore = player()->get<CScore>();
	if (pScore.score >= pScore.nextScoreThreshold)
	{
		auto temp = pScore.prevScoreThreshold;
		pScore.prevScoreThreshold = pScore.nextScoreThreshold;
		pScore.nextScoreThreshold += temp;

		pScore.level++;
	}
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
		player()->get<CState>().state = "running";
		pTransform.velocity = pTransform.velocity.normalize() * m_playerConfig.SPEED;
	}
	else
		player()->get<CState>().state = "idle";


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
		
		float newSpeed = eTransform.velocity.length() + eTransform.accel;
		eTransform.velocity = eTransform.velocity.normalize() * newSpeed;

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

		Vec2f desired = (tTransform.pos - eTransform.pos).normalize() * eFollow.speed;
		Vec2f steering = (desired - eTransform.velocity) * eFollow.steering_scale;
		eTransform.velocity += steering;
	}
}

void Scene_Play::sStatus()
{

}

void Scene_Play::applyKnockback(std::shared_ptr<Entity> target, const Vec2f& fromPos,
	float force, int duration) {
	auto& tTransform = target->get<CTransform>();

	Vec2f direction = (tTransform.pos - fromPos).normalize();
	auto& tKnockback = target->add<CKnockback>(force, duration);
	tKnockback.beingKnockedback = true;

	if (target->tag() == "enemy")
	{
		target->get<CState>().state = "knockback";
	}

	tTransform.velocity = direction * force / tTransform.scale;
	tTransform.accel = -0.2f * tTransform.scale;
}

void Scene_Play::sKnockback() {
	for (auto& e : m_entityManager.getEntities("enemy")) {
		if (!e->has<CKnockback>()) continue;

		auto& kb = e->get<CKnockback>();
		if (!kb.beingKnockedback) continue;

		if (kb.duration > 0)
		{
			kb.duration--;
		}
		else
		{
			auto& transform = e->get<CTransform>();
			transform.velocity = { 0, 0 };
			transform.accel = 0;
			e->remove<CKnockback>();

			if (e->get<CHealth>().health <= 0)
				e->get<CState>().state = "dead";
			else
				e->get<CState>().state = "alive";		
		}
	}
}

void Scene_Play::sCollision()
{
	for (auto& e1 : m_entityManager.getEntities("enemy"))
	{
		Vec2f overlap = Physics::GetOverlap(e1, player());
		if (overlap.x > 0 && overlap.y > 0)
		{
			auto& e1Health = e1->get<CHealth>().health;
			auto& playerHealth = player()->get<CHealth>().health;
			e1Health -= player()->get<CDamage>().damage;
			playerHealth -= e1->get<CDamage>().damage;

			auto& pKnockback = player()->get<CKnockback>();
			applyKnockback(e1, player()->get<CTransform>().pos,
				pKnockback.magnitude, pKnockback.duration);
			applyKnockback(player(), e1->get<CTransform>().pos,
				pKnockback.magnitude, pKnockback.duration);
			playSound("PlasticZap", 30);

			if (e1Health <= 0)
			{
				e1->get<CState>().state = "dead";
				player()->get<CScore>().score += e1->get<CScore>().score;
			}
			if (playerHealth <= 0)
			{
				player()->get<CState>().state = "dead";
				return;
			}	
		}

		for (auto& pAttack : m_entityManager.getEntities("playerAttack"))
		{
			overlap = Physics::GetOverlap(e1, pAttack);
			if (overlap.x > 0 && overlap.y > 0)
			{
				auto& e1Health = e1->get<CHealth>().health;
				auto& pAttackHealth = pAttack->get<CHealth>().health;
				auto& pDamage = pAttack->get<CDamage>().damage;
				e1Health -= pDamage;
				pAttackHealth -= e1->get<CDamage>().damage;

				auto& paKnockback = pAttack->get<CKnockback>();
				applyKnockback(e1, pAttack->get<CTransform>().pos,
					paKnockback.magnitude, paKnockback.duration);
				playSound("PlasticZap", 30);

				// damage number pop up
				sf::Text damageNumText(m_game->assets().getFont("FutureMillennium"));
				damageNumText.setString(std::to_string(pDamage));
				damageNumText.setCharacterSize(16);
				damageNumText.setOutlineColor(sf::Color(86, 106, 137));
				damageNumText.setOutlineThickness(0.5f);

				sf::FloatRect bounds = damageNumText.getLocalBounds();
				damageNumText.setOrigin({ bounds.position.x / 2.f, bounds.position.y / 2.f });
				auto& enemyPosition = e1->get<CTransform>().pos;
				damageNumText.setPosition(enemyPosition);

				auto disappearingText = m_entityManager.addEntity("disappearingText", "disappearingText");
				auto& dmComponent = disappearingText->add<CDisappearingText>(damageNumText);
				disappearingText->add<CTransform>(enemyPosition, dmComponent.velocity);
				disappearingText->add<CLifespan>(dmComponent.lifetime, m_currentFrame);

				if (e1Health <= 0)
				{
					e1->get<CState>().state = "dead";
					player()->get<CScore>().score += e1->get<CScore>().score;
				}
				if (pAttackHealth <= 0)
				{
					pAttack->destroy();
				}
			}
		}

		for (auto& e2 : m_entityManager.getEntities("enemy"))
		{
			if (e1->id() == e2->id())
				continue;

			overlap = Physics::GetOverlap(e1, e2);
			if (overlap.x > 0 && overlap.y > 0)
			{
				auto& e1Transform = e1->get<CTransform>();
				auto& e2Transform = e2->get<CTransform>();
				Vec2f prevOverlap = Physics::GetPreviousOverlap(e1, e2);
				if (prevOverlap.x > 0)
				{
					e1Transform.velocity.y = 0;
					if (e1Transform.prevPos.y < e2Transform.pos.y)
						e1Transform.pos.y -= overlap.y;
					else
						e1Transform.pos.y += overlap.y;
				}
				else if (prevOverlap.y > 0)
				{
					e1Transform.velocity.x = 0;
					if (e1Transform.prevPos.x < e2Transform.pos.x)
						e1Transform.pos.x -= overlap.x;
					else
						e1Transform.pos.x += overlap.x;
				}
			}
		}
	}

	for (auto& gem : m_entityManager.getEntities("gem"))
	{
		Vec2f overlap = Physics::GetOverlap(gem, player());
		if (overlap.x > 0 && overlap.y > 0)
		{
			player()->get<CScore>().score += gem->get<CScore>().score;
			gem->destroy();
			playSound("CoinZap", 15);
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

void Scene_Play::sDisappearingText()
{
	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CDisappearingText>()) continue;

		auto& lifespan = entity->get<CLifespan>();
		auto& dmg = entity->get<CDisappearingText>();

		float progress = static_cast<float>(m_currentFrame - lifespan.frameCreated) / lifespan.lifespan;
		if (progress > 1.f) progress = 1.f;
		if (progress < 0.f) progress = 0.f;

		int alpha = static_cast<int>(255.f * (1.f - progress));
		auto curColor = dmg.text.getFillColor();
		curColor.a = alpha;
		dmg.text.setFillColor(curColor);
		curColor = dmg.text.getOutlineColor();
		curColor.a = alpha;
		dmg.text.setOutlineColor(curColor);
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
	auto basicAttack = m_entityManager.addEntity("playerAttack", "basicAttack");

	float attackAngle = std::atan2(attackDir.y, attackDir.x) * 180.0f / 3.14159f;
	basicAttack->add<CTransform>(pTransform.pos + attackDir * pBasicAttack.distanceFromPlayer
		, Vec2f(0, 0), attackAngle);

	auto& baAnimation = basicAttack->add<CAnimation>(m_game->assets().getAnimation("BasicAttack"), true).animation;
	baAnimation.m_sprite.setScale({ pBasicAttack.scale, pBasicAttack.scale });

	basicAttack->add<CBoundingBox>(Vec2f(baAnimation.m_size.x, baAnimation.m_size.y / 2) * pBasicAttack.scale);
	basicAttack->add<CLifespan>(pBasicAttack.duration, m_currentFrame);
	basicAttack->add<CHealth>(pBasicAttack.health);
	basicAttack->add<CMoveAtSameVelocity>(player());
	basicAttack->add<CKnockback>(pBasicAttack.knockMagnitude, pBasicAttack.knockDuration);
	basicAttack->add<CDamage>(pBasicAttack.damage);

	playSound("SwordSlash", 30);
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
	auto specialAttack = m_entityManager.addEntity("playerAttack", "specialAttack");

	float attackAngle = std::atan2(attackDir.y, attackDir.x) * 180.0f / 3.14159f;
	auto& saTransform = specialAttack->add<CTransform>(pTransform.pos + attackDir,
		attackDir * pSpecialAttack.speed, attackAngle);
	saTransform.accel = pSpecialAttack.decel;

	auto& saAnimation = specialAttack->add<CAnimation>(m_game->assets().getAnimation("BasicAttack"), true).animation;
	saAnimation.m_sprite.setScale({ pSpecialAttack.scale, pSpecialAttack.scale });

	specialAttack->add<CBoundingBox>(Vec2f(saAnimation.m_size.x, saAnimation.m_size.y / 2) * pSpecialAttack.scale);
	specialAttack->add<CLifespan>(pSpecialAttack.duration, m_currentFrame);
	specialAttack->add<CHealth>(pSpecialAttack.health);
	specialAttack->add<CKnockback>(pSpecialAttack.knockMagnitude, pSpecialAttack.knockDuration);
	specialAttack->add<CDamage>(pSpecialAttack.damage);

	playSound("HighWhoosh", 50);
}

void Scene_Play::playSound(const std::string& name, float volume)
{
	auto& sound = m_game->assets().getSound(name);
	float pitch = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f; // range [0.8, 1.2]
	sound.setPitch(pitch);
	sound.setVolume(volume);
	sound.play();
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
		{
			if (!m_playerDied && entity->id() == player()->id())
			{
				// crashes for some reason if m_playerDied is set here
				// m_playerDied = true;
				// return;
			}
			else
			{
				entity->destroy();
				continue;
			}
		}

		if (entity->tag() == "player")
		{
			auto& pState = player()->get<CState>().state;
			auto& pAnimation = player()->get<CAnimation>().animation;
			if (pState == "idle" && pAnimation.m_name != "StormheadIdle")
			{
				player()->add<CAnimation>(m_game->assets().getAnimation("StormheadIdle"), true);
			}
			else if (pState == "running" && pAnimation.m_name != "StormheadRun")
			{
				player()->add<CAnimation>(m_game->assets().getAnimation("StormheadRun"), true);
			}
			else if (pState == "dead" && pAnimation.m_name != "StormheadDeath")
			{
				player()->add<CAnimation>(m_game->assets().getAnimation("StormheadDeath"), false);
				// temporarily set m_playerDied here instead
				m_playerDied = true;
				return;
			}
		}

		if (entity->tag() == "enemy")
		{
			if (entity->name() == "chainBot")
			{
				auto& eState = entity->get<CState>().state;
				if (eState == "alive" && entity->get<CAnimation>().animation.m_name != "ChainBotIdle")
				{
					auto& eAnimation = entity->add<CAnimation>(m_game->assets().getAnimation("ChainBotIdle"), true);
				}
				else if (eState == "knockback" && entity->get<CAnimation>().animation.m_name != "ChainBotHit")
				{
					auto& eAnimation = entity->add<CAnimation>(m_game->assets().getAnimation("ChainBotHit"), true);
				}
				else if (eState == "dead" && entity->get<CAnimation>().animation.m_name != "ChainBotDeath")
				{
					auto& eAnimation = entity->add<CAnimation>(m_game->assets().getAnimation("ChainBotDeath"), false);
					enemyDied(entity);
				}
			}
			else if (entity->name() == "botWheel")
			{
				auto& eState = entity->get<CState>().state;
				if (eState == "alive" && entity->get<CAnimation>().animation.m_name != "BotWheelRun")
				{
					auto& eAnimation = entity->add<CAnimation>(m_game->assets().getAnimation("BotWheelRun"), true);
				}
				else if (eState == "knockback" && entity->get<CAnimation>().animation.m_name != "BotWheelHit")
				{
					auto& eAnimation = entity->add<CAnimation>(m_game->assets().getAnimation("BotWheelHit"), true);
				}
				else if (eState == "dead" && entity->get<CAnimation>().animation.m_name != "BotWheelDead")
				{
					auto& eAnimation = entity->add<CAnimation>(m_game->assets().getAnimation("BotWheelDead"), false);
					enemyDied(entity);
				}
			}
		}
	}
}

void Scene_Play::enemyDied(std::shared_ptr<Entity> enemy)
{
	auto& eTransform = enemy->get<CTransform>();

	enemy->remove<CFollow>();
	enemy->remove<CBoundingBox>();
	eTransform.velocity = { 0, 0 };

	playSound("LaserPebble", 40);
	for (int i = 0; i < enemy->get<CScore>().score; i++)
	{
		spawnGem(eTransform.pos);
	}
}

void Scene_Play::sSound()
{
	auto& pState = player()->get<CState>().state;
	if (pState == "running" && m_currentFrame % 15 == 0)
	{
		playSound("BubbleStep", 5);
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
		if (!entity->has<CAnimation>()) continue;

		auto& transform = entity->get<CTransform>();
		auto& animation = entity->get<CAnimation>().animation;

		animation.m_sprite.setPosition(transform.pos);
		animation.m_sprite.setRotation(sf::degrees(transform.angle));
		animation.m_sprite.setScale(Vec2f(transform.scale, transform.scale));

		if (entity->id() != player()->id())
		{
			// Create a shadow sprite by copying the original
			sf::Sprite shadow = animation.m_sprite;
			shadow.move({ transform.scale * animation.m_size.x * 0.2f, transform.scale * animation.m_size.y * 0.2f });
			shadow.setColor(sf::Color(0, 0, 0, 60));
			shadow.setScale({ transform.scale, transform.scale * 0.3f });
			window.draw(shadow);
		}

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

		if (entity->tag() == "enemy" && entity->get<CState>().state == "alive")
		{
			// Bar settings
			float width = transform.scale * animation.m_size.x * 0.5f;
			float height = 3.f;
			auto& health = entity->get<CHealth>();
			float hpPercent = static_cast<float>(health.health) / health.maxHealth;

			sf::Vector2f barPos = transform.pos + sf::Vector2f(-width / 2, -transform.scale * animation.m_size.y / 2);

			// Background (gray)
			sf::RectangleShape bgBar(sf::Vector2f(width, height));
			bgBar.setFillColor(sf::Color(0, 0, 0, 60));
			bgBar.setPosition(barPos + sf::Vector2f(1.f, 1.f));

			// Health (green/red)
			sf::RectangleShape hpBar(sf::Vector2f(width * hpPercent, height));
			hpBar.setFillColor(sf::Color::White);
			hpBar.setPosition(barPos);

			// Draw both
			window.draw(bgBar);
			window.draw(hpBar);
		}
	}
	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CDisappearingText>()) continue;

		auto& eDisappearingText = entity->get<CDisappearingText>();
		auto& eTransform = entity->get<CTransform>();

		eDisappearingText.text.setPosition(eTransform.pos);
		window.draw(eDisappearingText.text);
	}

	window.setView(window.getDefaultView());

	auto& pScore = player()->get<CScore>();
	m_gridText.setString(std::to_string(pScore.score));
	window.draw(m_gridText);

	auto& pAnimation = player()->get<CAnimation>().animation;
	auto& pTransform = player()->get<CTransform>();

	// player health bar
	float healthBarWidth = 400.f;
	float HealthBarHeight = 30.f;
	auto& pHealth = player()->get<CHealth>();
	float hpPercent = static_cast<float>(pHealth.health) / pHealth.maxHealth;
	sf::Vector2f barPos = sf::Vector2f((width() - healthBarWidth) / 2, height() - 120);
	// Background (transparent black)
	sf::RectangleShape bgBar(sf::Vector2f(healthBarWidth, HealthBarHeight));
	bgBar.setFillColor(sf::Color(0, 0, 0, 60));
	bgBar.setPosition(barPos + sf::Vector2f(2.f, 2.f));
	// Health (white)
	sf::RectangleShape hpBar(sf::Vector2f(healthBarWidth * hpPercent, HealthBarHeight));
	hpBar.setFillColor(sf::Color::White);
	hpBar.setPosition(barPos);
	// Draw both
	window.draw(bgBar);
	window.draw(hpBar);
	//

	// player health text
	sf::Text scoreText(m_game->assets().getFont("FutureMillennium"));
	scoreText.setOutlineThickness(1.0f);
	scoreText.setOutlineColor(sf::Color(86, 106, 137));
	scoreText.setString(std::to_string(pHealth.health) + " / " + std::to_string(pHealth.maxHealth));
	sf::FloatRect bounds = scoreText.getLocalBounds();
	scoreText.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
	scoreText.setPosition(sf::Vector2f(width() / 2, height() - 105));

	window.draw(scoreText);
	//

	// circle score UI
	float percent = (pScore.score - pScore.prevScoreThreshold) /
		static_cast<float>(pScore.nextScoreThreshold - pScore.prevScoreThreshold);
	int segments = 100;
	float radius = 60.f;
	sf::Vector2f center(barPos.x - radius - 30.f, barPos.y + 10.f);

	// Draw full circle shadow behind the partial pie
	sf::CircleShape pieShadow(radius);
	pieShadow.setPosition(center - sf::Vector2f(radius, radius) + sf::Vector2f(2.f, 2.f)); // Offset by (2,2)
	pieShadow.setFillColor(sf::Color(0, 0, 0, 60)); // semi-transparent black

	// ---- Actual pie on top ----
	sf::VertexArray pie(sf::PrimitiveType::TriangleFan, segments + 2);
	pie[0].position = center;
	pie[0].color = sf::Color::White;

	for (int i = 0; i <= segments; ++i)
	{
		float angle = i * (2 * 3.14159f * percent) / segments - 3.14159f / 2;
		float x = center.x + std::cos(angle) * radius;
		float y = center.y + std::sin(angle) * radius;

		pie[i + 1].position = { x, y };
		pie[i + 1].color = sf::Color::White;
	}

	window.draw(pieShadow);
	window.draw(pie);
	//

	// player score text
	sf::Text pieText(m_game->assets().getFont("FutureMillennium"));
	scoreText.setOutlineThickness(1.0f);
	scoreText.setOutlineColor(sf::Color(86, 106, 137));
	scoreText.setString(std::to_string(static_cast<int>(percent * 100)) + "%");
	bounds = scoreText.getLocalBounds();
	scoreText.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
	scoreText.setPosition(center);

	window.draw(scoreText);

	window.setView(m_cameraView);
}