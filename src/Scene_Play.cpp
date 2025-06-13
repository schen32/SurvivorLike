#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "Scene_Pause.h"
#include "Scene_NewWeapon.h"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "ParticleSystem.hpp"
#include "Utils.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <math.h>

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
	registerAction(sf::Keyboard::Scan::P, "PAUSE");
	registerAction(sf::Keyboard::Scan::Escape, "ESCAPE");
	registerAction(sf::Keyboard::Scan::H, "DISPLAY_HITBOX");

	registerAction(sf::Keyboard::Scan::A, "LEFT");
	registerAction(sf::Keyboard::Scan::D, "RIGHT");
	registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");
	registerAction(sf::Keyboard::Scan::Left, "LEFT");
	registerAction(sf::Keyboard::Scan::Right, "RIGHT");
	registerAction(sf::Keyboard::Scan::Up, "UP");
	registerAction(sf::Keyboard::Scan::Down, "DOWN");

	registerAction(sf::Keyboard::Scan::Z, "TOGGLE_AUTO_AIM");
	registerAction(sf::Keyboard::Scan::X, "TOGGLE_AUTO_ATTACK");
	registerAction(sf::Keyboard::Scan::Q, "RING_ATTACK");
	registerAction(sf::Keyboard::Scan::R, "EXPLODE_ATTACK");
	registerAction(sf::Keyboard::Scan::E, "WHIRL_ATTACK");
	registerAction(sf::Keyboard::Scan::F, "BULLET_ATTACK");

	m_playerConfig = { 0, 0, 0, 0, 3.0f, 0, ""};

	m_gridText.setCharacterSize(100);
	m_gridText.setFont(m_game->assets().getFont("FutureMillennium"));
	m_gridText.setOutlineThickness(3.0f);
	m_gridText.setOutlineColor(sf::Color(86, 106, 137));
	m_gridText.setPosition({ 20, 10 });

	m_particleSystem.init(m_game->window().getSize());
	m_cameraView.setSize(sf::Vector2f(width(), height()));
	m_cameraView.zoom(0.5f);

	srand(static_cast<unsigned int>(time(nullptr)));
	std::vector<std::string> bgms = { "Awakened", "CargoHold", "TempleoftheValley",
		"Gaia'sGarden", "TempleoftheLake"};
	int randomIndex = rand() % bgms.size();

	m_musicName = bgms[randomIndex];
	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.setVolume(10);
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
	p->add<CAttractor>(50.0f, 150.0f);

	p->add<CBasicAttack>(m_currentFrame);
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
	static const int enemySpawnInterval = 45;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * height() / 2;

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
	static const int enemySpawnInterval = 60;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * height() / 2;

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
	static const int enemySpawnInterval = 750;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * height() / 2;

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
	static const int enemySpawnInterval = 900;
	if (m_currentFrame - lastEnemySpawnTime > enemySpawnInterval)
	{
		lastEnemySpawnTime = m_currentFrame;

		int spawnAngle = rand() % 360;
		Vec2f spawnPoint = Vec2f(std::cos(spawnAngle), std::sin(spawnAngle)) * height() / 2;

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
		sAI();
		sKnockback();
		sAttraction();
		sMovement();
		sCollision();
		sScore();
		sSound();
		sCamera();
		sAnimation();
	}

	if (m_playerDied)
	{
		onExitScene();
		m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
	}
}

void Scene_Play::sScore()
{
	auto& pScore = player()->get<CScore>();
	if (pScore.score >= pScore.nextScoreThreshold)
	{
		auto temp = pScore.prevScoreThreshold;
		pScore.prevScoreThreshold = pScore.nextScoreThreshold;
		pScore.nextScoreThreshold += std::max(temp, 100);

		pScore.level++;

		onExitScene();
		m_game->changeScene("NEW_WEAPON", std::make_shared<Scene_NewWeapon>(m_game, player()));
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

bool Scene_Play::applyDamage(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2)
{
	auto& e1Health = e1->get<CHealth>();
	auto& e2Health = e2->get<CHealth>();
	if (m_currentFrame - e1Health.lastTakenDamage < e1Health.invulTime)
	{
		return false;
	}
	e1Health.lastTakenDamage = m_currentFrame;

	e1Health.health -= e2->get<CDamage>().damage;
	e2Health.health -= e1->get<CDamage>().damage;

	if (e2->has<CKnockback>())
	{
		auto& paKnockback = e2->get<CKnockback>();
		applyKnockback(e1, e2->get<CTransform>().pos, paKnockback.magnitude, paKnockback.duration);
	}
	
	spawnDisappearingText(std::to_string(e2->get<CDamage>().damage), e1->get<CTransform>().pos);
	playSound("PlasticZap", 30);
	return true;
}


void Scene_Play::sCollision()
{
	for (auto& e1 : m_entityManager.getEntities("enemy"))
	{
		Vec2f overlap = Physics::GetOverlap(e1, player());
		if (overlap.x > 0 && overlap.y > 0)
		{
			if (!applyDamage(e1, player())) continue;

			if (e1->get<CHealth>().health <= 0)
			{
				e1->get<CState>().state = "dead";
				player()->get<CScore>().score += e1->get<CScore>().score;
			}
			if (player()->get<CHealth>().health <= 0)
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
				if (!applyDamage(e1, pAttack)) continue;
			
				if (e1->get<CHealth>().health <= 0)
				{
					e1->get<CState>().state = "dead";
					player()->get<CScore>().score += e1->get<CScore>().score;
				}
				if (pAttack->get<CHealth>().health <= 0)
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
			auto& pScore = player()->get<CScore>().score;
			auto& gemScore = gem->get<CScore>().score;
			pScore += gemScore;
			spawnDisappearingText("+" + std::to_string(gemScore), gem->get<CTransform>().pos);
			playSound("CoinZap", 15);
			gem->destroy();
		}
	}
}

void Scene_Play::spawnDisappearingText(const std::string& text, const Vec2f& pos) {
	// damage number pop up
	sf::Text damageNumText(m_game->assets().getFont("FutureMillennium"));
	damageNumText.setString(text);
	damageNumText.setCharacterSize(16);
	damageNumText.setOutlineColor(sf::Color(86, 106, 137));
	damageNumText.setOutlineThickness(0.5f);

	sf::FloatRect bounds = damageNumText.getLocalBounds();
	damageNumText.setOrigin({ bounds.position.x / 2.f, bounds.position.y / 2.f });
	auto& enemyPosition = pos;
	damageNumText.setPosition(enemyPosition);

	auto disappearingText = m_entityManager.addEntity("disappearingText", "disappearingText");
	auto& dmComponent = disappearingText->add<CDisappearingText>(damageNumText);
	disappearingText->add<CTransform>(enemyPosition, dmComponent.velocity);
	disappearingText->add<CLifespan>(dmComponent.lifetime, m_currentFrame);
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
	Vec2f attackPos = Vec2f(0, 0);
	if (pInput.autoAim)
	{
		auto nearestEnemy = getNearestEnemy(player());
		if (nearestEnemy)
			attackPos = nearestEnemy->get<CTransform>().pos;
	}
	else
	{
		attackPos = m_mousePos;
	}

	if (pInput.autoAttack)
	{
		spawnBasicAttack(attackPos);
		spawnSpecialAttack(attackPos);
		spawnRingAttack(player()->get<CTransform>().pos);
		spawnExplodeAttack(attackPos);
		spawnWhirlAttack(attackPos);
		spawnBulletAttack(attackPos);
	}
	else
	{
		if (pInput.basicAttack)
			spawnBasicAttack(attackPos);
		if (pInput.specialAttack)
			spawnSpecialAttack(attackPos);
		if (pInput.ringAttack)
			spawnRingAttack(player()->get<CTransform>().pos);
		if (pInput.explodeAttack)
			spawnExplodeAttack(attackPos);
		if (pInput.whirlAttack)
			spawnWhirlAttack(attackPos);
		if (pInput.bulletAttack)
			spawnBulletAttack(attackPos);
	}
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

	auto& baAnimation = basicAttack->add<CAnimation>(m_game->assets().getAnimation("Slash1"), true).animation;
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

	auto& saAnimation = specialAttack->add<CAnimation>(m_game->assets().getAnimation("Slash1"), true).animation;
	saAnimation.m_sprite.setScale({ pSpecialAttack.scale, pSpecialAttack.scale });

	specialAttack->add<CBoundingBox>(Vec2f(saAnimation.m_size.x, saAnimation.m_size.y / 2) * pSpecialAttack.scale);
	specialAttack->add<CLifespan>(pSpecialAttack.duration, m_currentFrame);
	specialAttack->add<CHealth>(pSpecialAttack.health);
	specialAttack->add<CKnockback>(pSpecialAttack.knockMagnitude, pSpecialAttack.knockDuration);
	specialAttack->add<CDamage>(pSpecialAttack.damage);

	playSound("HighWhoosh", 50);
}

void Scene_Play::spawnBulletAttack(const Vec2f& targetPos)
{
	if (!player()->has<CBulletAttack>())
		return;

	auto& pBulletAttack = player()->get<CBulletAttack>();
	if ((m_currentFrame - pBulletAttack.lastAttackTime) < pBulletAttack.cooldown)
		return;
	pBulletAttack.lastAttackTime = m_currentFrame;

	auto& pTransform = player()->get<CTransform>();

	Vec2f attackDir = (targetPos - pTransform.pos).normalize();
	auto bulletAttack = m_entityManager.addEntity("playerAttack", "bulletAttack");

	float attackAngle = std::atan2(attackDir.y, attackDir.x) * 180.0f / 3.14159f;
	auto& bulletTransform = bulletAttack->add<CTransform>(pTransform.pos + attackDir,
		attackDir * pBulletAttack.speed, attackAngle);
	bulletTransform.accel = pBulletAttack.decel;
	bulletTransform.scale = pBulletAttack.scale;

	auto& saAnimation = bulletAttack->add<CAnimation>(m_game->assets().getAnimation("Bullet1"), true).animation;
	saAnimation.m_sprite.setScale({ pBulletAttack.scale, pBulletAttack.scale });

	bulletAttack->add<CBoundingBox>(Vec2f(saAnimation.m_size.x, saAnimation.m_size.y / 4) * pBulletAttack.scale);
	bulletAttack->add<CLifespan>(pBulletAttack.duration, m_currentFrame);
	bulletAttack->add<CHealth>(pBulletAttack.health);
	bulletAttack->add<CKnockback>(pBulletAttack.knockMagnitude, pBulletAttack.knockDuration);
	bulletAttack->add<CDamage>(pBulletAttack.damage);

	playSound("LaserShot", 5);
}

void Scene_Play::spawnRingAttack(const Vec2f& targetPos)
{
	if (!player()->has<CRingAttack>())
		return;

	auto& pRingAttack = player()->get<CRingAttack>();
	if ((m_currentFrame - pRingAttack.lastAttackTime) < pRingAttack.cooldown)
		return;
	pRingAttack.lastAttackTime = m_currentFrame;

	auto ringAttack = m_entityManager.addEntity("playerAttack", "ringAttack");
	auto& ringTransform = ringAttack->add<CTransform>(targetPos);
	ringTransform.scale = pRingAttack.scale;
	auto& ringAnimation = ringAttack->add<CAnimation>(m_game->assets().getAnimation("Ring1"), true).animation;

	ringAttack->add<CBoundingBox>(Vec2f(ringAnimation.m_size.x, ringAnimation.m_size.y) * pRingAttack.scale);
	ringAttack->add<CLifespan>(pRingAttack.duration, m_currentFrame);
	ringAttack->add<CHealth>(pRingAttack.health);
	ringAttack->add<CDamage>(pRingAttack.damage);
	ringAttack->add<CMoveAtSameVelocity>(player());
	ringAttack->add<CKnockback>(pRingAttack.knockMagnitude, pRingAttack.knockDuration);

	playSound("FireSphere", 30);
}

void Scene_Play::spawnExplodeAttack(const Vec2f& targetPos)
{
	if (!player()->has<CExplodeAttack>())
		return;

	auto& pExplodeAttack = player()->get<CExplodeAttack>();
	if ((m_currentFrame - pExplodeAttack.lastAttackTime) < pExplodeAttack.cooldown)
		return;
	pExplodeAttack.lastAttackTime = m_currentFrame;

	auto explodeAttack = m_entityManager.addEntity("playerAttack", "explodeAttack");
	auto& ringTransform = explodeAttack->add<CTransform>(targetPos);
	ringTransform.scale = pExplodeAttack.scale;
	auto& ringAnimation = explodeAttack->add<CAnimation>(m_game->assets().getAnimation("Explode1"), true).animation;

	explodeAttack->add<CBoundingBox>(Vec2f(ringAnimation.m_size.x, ringAnimation.m_size.y) * pExplodeAttack.scale);
	explodeAttack->add<CLifespan>(pExplodeAttack.duration, m_currentFrame);
	explodeAttack->add<CHealth>(pExplodeAttack.health);
	explodeAttack->add<CDamage>(pExplodeAttack.damage);
	explodeAttack->add<CKnockback>(pExplodeAttack.knockMagnitude, pExplodeAttack.knockDuration);

	playSound("FireHit", 50);
}

void Scene_Play::spawnWhirlAttack(const Vec2f& targetPos)
{
	if (!player()->has<CWhirlAttack>())
		return;

	auto& pWhirlAttack = player()->get<CWhirlAttack>();
	if ((m_currentFrame - pWhirlAttack.lastAttackTime) < pWhirlAttack.cooldown)
		return;
	pWhirlAttack.lastAttackTime = m_currentFrame;

	auto whirlAttack = m_entityManager.addEntity("playerAttack", "whirlAttack");
	auto& ringTransform = whirlAttack->add<CTransform>(targetPos);
	ringTransform.scale = pWhirlAttack.scale;
	auto& ringAnimation = whirlAttack->add<CAnimation>(m_game->assets().getAnimation("Ring2"), true).animation;

	whirlAttack->add<CBoundingBox>(Vec2f(ringAnimation.m_size.x, ringAnimation.m_size.y) * pWhirlAttack.scale);
	whirlAttack->add<CLifespan>(pWhirlAttack.duration, m_currentFrame);
	whirlAttack->add<CHealth>(pWhirlAttack.health);
	whirlAttack->add<CDamage>(pWhirlAttack.damage);
	whirlAttack->add<CAttractor>(pWhirlAttack.attractStrength, pWhirlAttack.attractRadius);
	whirlAttack->add<CKnockback>(pWhirlAttack.knockMagnitude, pWhirlAttack.knockDuration);

	playSound("FireHit", 50);
}

void Scene_Play::sAttraction()
{
	for (auto& attractor : m_entityManager.getEntities("playerAttack")) {
		if (!attractor->has<CAttractor>()) continue;

		for (auto& target : m_entityManager.getEntities("enemy"))
		{
			applyAttraction(attractor, target);
		}
	}

	if (player()->has<CAttractor>())
	{
		for (auto& gem : m_entityManager.getEntities("gem"))
		{
			if (!applyAttraction(player(), gem))
			{
				gem->get<CTransform>().velocity = Vec2f(0, 0);
			}
		}
	}	
}

bool Scene_Play::applyAttraction(std::shared_ptr<Entity> attractor, std::shared_ptr<Entity> target) {
	auto& tTransform = target->get<CTransform>();
	auto& aTransform = attractor->get<CTransform>();
	auto& attract = attractor->get<CAttractor>();

	Vec2f diff = aTransform.pos - tTransform.pos;
	float distanceSquared = diff.lengthSquared();

	if (distanceSquared < 100.f)
	{
		tTransform.velocity = Vec2f(0, 0);
		return true;
	}
	else if (distanceSquared < attract.radius * attract.radius)
	{
		float distance = std::sqrt(distanceSquared);
		Vec2f direction = diff / distance;
		float force = attract.strength / distance;
		tTransform.velocity = direction * force;
		return true;
	}
	else
	{
		return false;
	}
}

void Scene_Play::sDoAction(const Action& action)
{
	auto& pInput = player()->get<CInput>();
	if (action.m_type == "START")
	{
		if (action.m_name == "LEFT")
			pInput.left = true;
		else if (action.m_name == "RIGHT")
			pInput.right = true;
		else if (action.m_name == "UP")
			pInput.up = true;
		else if (action.m_name == "DOWN")
			pInput.down = true;
		else if (action.m_name == "ESCAPE")
		{
			onExitScene();
			m_game->changeScene("PAUSE", std::make_shared<Scene_Pause>(m_game));
		}	
		else if (action.m_name == "PAUSE")
		{
			onExitScene();
			m_game->changeScene("NEW_WEAPON", std::make_shared<Scene_NewWeapon>(m_game, player()));
		}
		else if (action.m_name == "DISPLAY_HITBOX")
			pInput.displayHitbox = !pInput.displayHitbox;
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
			m_mousePos = m_game->window().mapPixelToCoords(action.m_mousePos);
		else if (action.m_name == "TOGGLE_AUTO_ATTACK")
			pInput.autoAttack = !pInput.autoAttack;
		else if (action.m_name == "TOGGLE_AUTO_AIM")
			pInput.autoAim = !pInput.autoAim;
		else if (action.m_name == "RING_ATTACK")
			pInput.ringAttack = !pInput.ringAttack;
		else if (action.m_name == "EXPLODE_ATTACK")
			pInput.explodeAttack = true;
		else if (action.m_name == "WHIRL_ATTACK")
			pInput.whirlAttack = true;
		else if (action.m_name == "BULLET_ATTACK")
			pInput.bulletAttack = true;
	}
	else if (action.m_type == "END")
	{
		if (action.m_name == "LEFT")
			pInput.left = false;
		else if (action.m_name == "RIGHT")
			pInput.right = false;
		if (action.m_name == "UP")
			pInput.up = false;
		else if (action.m_name == "DOWN")
			pInput.down = false;
		else if (action.m_name == "LEFT_CLICK")
			pInput.basicAttack = false;
		else if (action.m_name == "RIGHT_CLICK")
			pInput.specialAttack = false;
		else if (action.m_name == "EXPLODE_ATTACK")
			pInput.explodeAttack = false;
		else if (action.m_name == "WHIRL_ATTACK")
			pInput.whirlAttack = false;
		else if (action.m_name == "BULLET_ATTACK")
			pInput.bulletAttack = false;
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
				m_playerDied = true;
				return;
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

void Scene_Play::onExitScene()
{
	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.pause();
}

void Scene_Play::onEnterScene()
{
	auto& window = m_game->window();
	window.setView(m_cameraView);

	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.play();

	player()->add<CInput>();
}

void Scene_Play::sGui()
{

}

void Scene_Play::renderShadow(std::shared_ptr<Entity> entity)
{
	// Create a shadow sprite by copying the original
	auto& animation = entity->get<CAnimation>().animation;
	auto& transform = entity->get<CTransform>();
	sf::Sprite shadow = animation.m_sprite;
	shadow.move({ transform.scale * animation.m_size.x * 0.2f, transform.scale * animation.m_size.y * 0.2f });
	shadow.setColor(sf::Color(0, 0, 0, 60));
	shadow.setScale({ transform.scale, transform.scale * 0.3f });
	m_game->window().draw(shadow);
}


void Scene_Play::sRender()
{
	auto& window = m_game->window();
	window.clear(sf::Color(204, 226, 225));

	for (auto& entity : m_entityManager.getEntities("gem"))
	{
		auto& transform = entity->get<CTransform>();
		auto& animation = entity->get<CAnimation>().animation;

		animation.m_sprite.setPosition(transform.pos);
		renderShadow(entity);
		window.draw(animation.m_sprite);
	}

	for (auto& entity : m_entityManager.getEntities("enemy"))
	{
		auto& transform = entity->get<CTransform>();
		auto& animation = entity->get<CAnimation>().animation;

		animation.m_sprite.setPosition(transform.pos);
		animation.m_sprite.setRotation(sf::degrees(transform.angle));
		animation.m_sprite.setScale(Vec2f(transform.scale, transform.scale));

		renderShadow(entity);
		window.draw(animation.m_sprite);

		if (entity->get<CState>().state == "alive")
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
	for (auto& entity : m_entityManager.getEntities("playerAttack"))
	{
		auto& transform = entity->get<CTransform>();
		auto& animation = entity->get<CAnimation>().animation;

		animation.m_sprite.setPosition(transform.pos);
		animation.m_sprite.setRotation(sf::degrees(transform.angle));
		animation.m_sprite.setScale(Vec2f(transform.scale, transform.scale));

		renderShadow(entity);
		window.draw(animation.m_sprite);
	}
	// draw player
	auto& transform = player()->get<CTransform>();
	auto& animation = player()->get<CAnimation>().animation;

	animation.m_sprite.setPosition(transform.pos);
	animation.m_sprite.setScale(Vec2f(transform.scale, transform.scale));

	window.draw(animation.m_sprite);

	if (player()->get<CInput>().displayHitbox)
	{
		for (auto& entity : m_entityManager.getEntities())
		{
			if (!entity->has<CBoundingBox>()) continue;
			
			auto& transform = entity->get<CTransform>();
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
	
	for (auto& entity : m_entityManager.getEntities("disappearingText"))
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
	float healthBarWidth = 500.f;
	float healthBarHeight = 50.f;
	auto& pHealth = player()->get<CHealth>();
	float hpPercent = static_cast<float>(pHealth.health) / pHealth.maxHealth;
	sf::Vector2f barPos = sf::Vector2f(width() / 2, height() - 150);
	// Background (transparent black)
	sf::RectangleShape bgBar(sf::Vector2f(healthBarWidth, healthBarHeight));
	bgBar.setOrigin(sf::Vector2f(healthBarWidth, healthBarHeight) / 2.f);
	bgBar.setFillColor(sf::Color(0, 0, 0, 60));
	bgBar.setPosition(barPos + sf::Vector2f(2.f, 2.f));
	// Health (white)
	sf::RectangleShape hpBar(sf::Vector2f(healthBarWidth * hpPercent, healthBarHeight));
	hpBar.setOrigin(sf::Vector2f(healthBarWidth, healthBarHeight) / 2.f);
	hpBar.setFillColor(sf::Color::White);
	hpBar.setPosition(barPos);
	// Draw both
	window.draw(bgBar);
	window.draw(hpBar);
	//

	// player health text
	sf::Text scoreText(m_game->assets().getFont("FutureMillennium"));
	scoreText.setCharacterSize(40.f);
	scoreText.setOutlineThickness(1.0f);
	scoreText.setOutlineColor(sf::Color(86, 106, 137));
	scoreText.setString(std::to_string(pHealth.health) + " / " + std::to_string(pHealth.maxHealth));
	sf::FloatRect bounds = scoreText.getLocalBounds();
	scoreText.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
	scoreText.setPosition(barPos);

	window.draw(scoreText);
	//

	// circle score UI
	float percent = (pScore.score - pScore.prevScoreThreshold) /
		static_cast<float>(pScore.nextScoreThreshold - pScore.prevScoreThreshold);
	int segments = 100;
	float radius = 120.f;
	sf::Vector2f center(barPos.x - radius * 3.5, barPos.y);

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