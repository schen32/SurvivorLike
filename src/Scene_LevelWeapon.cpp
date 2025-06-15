#include "Scene_LevelWeapon.h"
#include "Scene_Play.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "Utils.hpp"

#include <iostream>
#include <random>
#include <algorithm>

Scene_LevelWeapon::Scene_LevelWeapon(GameEngine* gameEngine, std::shared_ptr<Entity> player)
	: Scene(gameEngine)
{
	init(player);
}

void Scene_LevelWeapon::init(std::shared_ptr<Entity> player)
{
	/*registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");
	registerAction(sf::Keyboard::Scan::D, "PLAY");
	registerAction(sf::Keyboard::Scan::Escape, "QUIT");*/

	m_musicName = "Battle3";
	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.setVolume(20);
	bgm.setLooping(true);
	bgm.play();

	auto& window = m_game->window();
	sf::Vector2f windowSize = sf::Vector2f(width(), height());
	m_menuView.setCenter(windowSize / 2.0f);
	m_menuView.setSize(windowSize);
	m_menuView.setViewport(sf::FloatRect({ 0.2f, 0 }, { 0.6f, 1.0f }));
	window.setView(m_menuView);

	m_weaponMap.insert({ "MeeleSlash",
		{ m_game->assets().getAnimation("Slash1"), "Close Slash", "-cooldown +size, damage"} });
	m_weaponMap.insert({ "RangedSlash",
		{m_game->assets().getAnimation("Slash1"), "Far Slash", "-cooldown +size, speed, damage"} });
	m_weaponMap.insert({ "FireRing",
		{ m_game->assets().getAnimation("Ring1"), "Ring of Fire", "-cooldown +size, damage"} });
	m_weaponMap.insert({ "Whirlpool",
		{ m_game->assets().getAnimation("Ring2"), "Whirlpool", "-cooldown +size, damage"} });
	m_weaponMap.insert({ "Explosion",
		{ m_game->assets().getAnimation("Explode1"), "Explosion", "-cooldown +size, damage"} });
	m_weaponMap.insert({ "LaserBullet",
		{ m_game->assets().getAnimation("Bullet1"), "Laser Bullet", "-cooldown +size, speed, damage"} });

	m_player = player;
	std::vector<std::string> playerWeapons;
	if (player->has<CBasicAttack>() && player->get<CBasicAttack>().level < 10)
		playerWeapons.push_back("MeeleSlash");
	if (player->has<CSpecialAttack>() && player->get<CSpecialAttack>().level < 10)
		playerWeapons.push_back("RangedSlash");
	if (player->has<CRingAttack>() && player->get<CRingAttack>().level < 10)
		playerWeapons.push_back("FireRing");
	if (player->has<CWhirlAttack>() && player->get<CWhirlAttack>().level < 10)
		playerWeapons.push_back("Whirlpool");
	if (player->has<CExplodeAttack>() && player->get<CExplodeAttack>().level < 10)
		playerWeapons.push_back("Explosion");
	if (player->has<CBulletAttack>() && player->get<CBulletAttack>().level < 10)
		playerWeapons.push_back("LaserBullet");

	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(playerWeapons.begin(), playerWeapons.end(), gen);

	size_t count = std::min<size_t>(3, playerWeapons.size());
	std::vector<std::string> weapons(playerWeapons.begin(), playerWeapons.begin() + count);
	loadScene(weapons);
}

void Scene_LevelWeapon::loadScene(const std::vector<std::string>& weapons)
{
	m_entityManager = EntityManager();

	auto title = m_entityManager.addEntity("ui", "Choose Upgrade");
	auto& tAnimation = title->add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true).animation;
	tAnimation.m_sprite.setScale(sf::Vector2f(2.f, 0.6f));
	title->add<CTransform>(Vec2f(width() / 2, height() * 0.10f));

	for (size_t i = 0; i < weapons.size(); i++)
	{
		auto button = m_entityManager.addEntity("button", weapons[i]);
		button->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
		auto& firstTransform = button->add<CTransform>(Vec2f(width() / 2, height() * (0.35f + 0.25 * i)));
		button->add<CState>("unselected");
	}
}

void Scene_LevelWeapon::update()
{
	m_entityManager.update();
	sHover();
	sAnimation();
}

void Scene_LevelWeapon::sHover()
{
	for (auto& button : m_entityManager.getEntities("button"))
	{
		auto& buttonState = button->get<CState>().state;
		if (Utils::IsInside(m_mousePos, button))
			buttonState = "selected";
		else
			buttonState = "unselected";
	}
}

void Scene_LevelWeapon::sAnimation()
{
	for (auto& button : m_entityManager.getEntities("button"))
	{
		auto& buttonState = button->get<CState>().state;
		auto& buttonAnimation = button->get<CAnimation>().animation;

		if (buttonState == "selected" && buttonAnimation.m_name != "ButtonHover")
		{
			button->add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true);
			playSound("BubblierStep", 15);
		}
		else if (buttonState == "unselected" && buttonAnimation.m_name != "Button")
		{
			button->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
		}
	}
}

void Scene_LevelWeapon::onEnd()
{
	m_game->quit();
}

void Scene_LevelWeapon::onExitScene()
{
	m_selectedIndex = 0;
	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.stop();
}

void Scene_LevelWeapon::onEnterScene()
{
	auto& window = m_game->window();
	window.setView(m_menuView);

	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.play();
}

void Scene_LevelWeapon::select()
{
	for (auto& button : m_entityManager.getEntities("button"))
	{
		if (!Utils::IsInside(m_mousePos, button)) continue;
		if (!m_game->changeScene("PLAY", nullptr)) continue;

		if (button->name() == "MeeleSlash")
		{
			auto& attack = m_player->get<CBasicAttack>();
			attack.cooldown -= 3;
			attack.scale += 0.1f;
			attack.duration += 3;
			attack.damage += 5 * attack.level;
			attack.health += 10;
			attack.knockMagnitude += 0.5f;
			attack.level++;
		}
		else if (button->name() == "RangedSlash")
		{
			auto& attack = m_player->get<CSpecialAttack>();
			attack.cooldown -= 15;
			attack.scale += 0.1f;
			attack.duration += 6;
			attack.speed += 1;
			attack.damage += 5 * attack.level;
			attack.health += 15;
			attack.knockMagnitude += 0.5f;
			attack.level++;
		}
			
		else if (button->name() == "FireRing")
		{
			auto& attack = m_player->get<CRingAttack>();
			attack.cooldown -= 20;
			attack.scale += 0.2f;
			attack.damage += 5 * attack.level;
			attack.health += 100;
			attack.knockMagnitude += 0.5f;
			attack.level++;
		}
			
		else if (button->name() == "Whirlpool")
		{
			auto& attack = m_player->get<CWhirlAttack>();
			attack.cooldown -= 25;
			attack.scale += 0.2f;
			attack.duration += 25;
			attack.damage += 5 * attack.level;
			attack.health += 100;
			attack.attractRadius += 15.0f;
			attack.level++;
		}
			
		else if (button->name() == "Explosion")
		{
			auto& attack = m_player->get<CExplodeAttack>();
			attack.cooldown -= 6;
			attack.scale += 0.2f;
			attack.duration += 6;
			attack.damage += 10 * attack.level;
			attack.health += 10;
			attack.knockMagnitude += 1.5f;
			attack.level++;
		}
			
		else if (button->name() == "LaserBullet")
		{
			auto& attack = m_player->get<CBulletAttack>();
			attack.cooldown -= 3;
			attack.scale += 0.2f;
			attack.duration += 3;
			attack.speed += 1;
			attack.damage += 5 * attack.level;
			attack.health += 5;
			attack.level++;
		}

		onExitScene();
	}
}

void Scene_LevelWeapon::sDoAction(const Action& action)
{
	if (action.m_type == "START")
	{
		/*if (action.m_name == "PLAY")
		{
			if (m_selectedMenuIndex == 0)
			{
				if (m_game->changeScene("PLAY",
					std::make_shared<Scene_Play>(m_game, m_levelPaths[m_selectedMenuIndex])))
					onPause();
			}
			else if (m_selectedMenuIndex == 1)
			{
				if (m_game->changeScene("PLAY", nullptr))
					onPause();
			}
		}
		else if (action.m_name == "UP")
		{
			if (m_selectedMenuIndex > 0)
				--m_selectedMenuIndex;
			else
				m_selectedMenuIndex = m_menuStrings.size() - 1;
		}
		else if (action.m_name == "DOWN")
		{
			m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
		}*/
		if (action.m_name == "QUIT")
		{
			onEnd();
		}
		else if (action.m_name == "MOUSE_MOVE")
			m_mousePos = action.m_mousePos;
		else if (action.m_name == "LEFT_CLICK")
		{
			m_mousePos = action.m_mousePos;
			select();
		}
	}
}

void Scene_LevelWeapon::sRender()
{
	auto& window = m_game->window();

	// Draw a solid background over just that portion
	sf::RectangleShape screenBackground(sf::Vector2f(width(), height()));
	screenBackground.setFillColor(sf::Color(204, 226, 225));
	window.draw(screenBackground);


	for (auto& entity : m_entityManager.getEntities("ui"))
	{
		auto& animation = entity->get<CAnimation>().animation;
		auto& transform = entity->get<CTransform>();

		animation.m_sprite.setPosition(transform.pos);
		window.draw(animation.m_sprite);

		auto buttonText = sf::Text(m_game->assets().getFont("ByteBounce"));
		buttonText.setCharacterSize(250 * transform.scale);
		buttonText.setString(entity->name());
		buttonText.setOutlineThickness(2.0f * transform.scale);
		buttonText.setOutlineColor(sf::Color(86, 106, 137));
		auto bounds = buttonText.getLocalBounds();
		buttonText.setOrigin(bounds.position + bounds.size / 2.f);
		buttonText.setPosition(transform.pos);
		window.draw(buttonText);
	}

	for (auto& entity : m_entityManager.getEntities("button"))
	{
		auto& animation = entity->get<CAnimation>().animation;
		auto& transform = entity->get<CTransform>();

		animation.m_sprite.setPosition(transform.pos);
		animation.m_sprite.setScale({ 1.8f, 0.95f });
		window.draw(animation.m_sprite);

		auto buttonBounds = animation.m_sprite.getGlobalBounds();

		WeaponData& weaponData = m_weaponMap.at(entity->name());
		weaponData.animation.update();
		weaponData.animation.m_sprite.setPosition(sf::Vector2f(
			transform.pos.x - buttonBounds.size.x / 3, transform.pos.y));
		weaponData.animation.m_sprite.setScale({ 3, 3 });
		window.draw(weaponData.animation.m_sprite);

		auto buttonText = sf::Text(m_game->assets().getFont("ByteBounce"));
		buttonText.setCharacterSize(200);
		buttonText.setString(weaponData.name);
		buttonText.setOutlineThickness(2.0f);
		buttonText.setOutlineColor(sf::Color(86, 106, 137));
		auto bounds = buttonText.getLocalBounds();
		buttonText.setOrigin(bounds.position + bounds.size / 2.f);
		buttonText.setPosition(sf::Vector2f(
			transform.pos.x + buttonBounds.size.x / 10, transform.pos.y - buttonBounds.size.y / 8));
		window.draw(buttonText);

		auto descriptText = sf::Text(m_game->assets().getFont("ByteBounce"));
		descriptText.setCharacterSize(100);
		descriptText.setString(weaponData.description);
		descriptText.setOutlineThickness(2.0f);
		descriptText.setOutlineColor(sf::Color(86, 106, 137));
		bounds = descriptText.getLocalBounds();
		descriptText.setOrigin(bounds.position + bounds.size / 2.f);
		descriptText.setPosition(sf::Vector2f(
			transform.pos.x + buttonBounds.size.x / 10, transform.pos.y + buttonBounds.size.y / 8));
		window.draw(descriptText);
	}
}