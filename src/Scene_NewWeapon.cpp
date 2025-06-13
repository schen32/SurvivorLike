#include "Scene_NewWeapon.h"
#include "Scene_Play.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "Utils.hpp"

#include <iostream>
#include <random>
#include <algorithm>

Scene_NewWeapon::Scene_NewWeapon(GameEngine* gameEngine, std::shared_ptr<Entity> player)
	: Scene(gameEngine)
{
	init(player);
}

void Scene_NewWeapon::init(std::shared_ptr<Entity> player)
{
	/*registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");
	registerAction(sf::Keyboard::Scan::D, "PLAY");
	registerAction(sf::Keyboard::Scan::Escape, "QUIT");*/

	m_musicName = "Challenge2";
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
		{ m_game->assets().getAnimation("Slash1"), "Close Slash", "Meele-ranged slash"} });
	m_weaponMap.insert({ "RangedSlash",
		{m_game->assets().getAnimation("Slash1"), "Far Slash", "Ranged flying slash attack"} });
	m_weaponMap.insert({ "FireRing",
		{ m_game->assets().getAnimation("Ring1"), "Ring of Fire", "Ring of fire that protects you"} });
	m_weaponMap.insert({ "Whirlpool",
		{ m_game->assets().getAnimation("Ring2"), "Whirlpool", "Whirlpool that sucks in enemies"} });
	m_weaponMap.insert({ "Explosion",
		{ m_game->assets().getAnimation("Explode1"), "Explosion", "Explosion attack with great knockback"} });
	m_weaponMap.insert({ "LaserBullet",
		{ m_game->assets().getAnimation("Bullet1"), "Laser Bullet", "Laser bullets with a high fire rate"} });

	m_player = player;
	std::vector<std::string> playerWeapons;
	if (!player->has<CBasicAttack>())
		playerWeapons.push_back("MeeleSlash");
	if (!player->has<CSpecialAttack>())
		playerWeapons.push_back("RangedSlash");
	if (!player->has<CRingAttack>())
		playerWeapons.push_back("FireRing");
	if (!player->has<CWhirlAttack>())
		playerWeapons.push_back("Whirlpool");
	if (!player->has<CExplodeAttack>())
		playerWeapons.push_back("Explosion");
	if (!player->has<CBulletAttack>())
		playerWeapons.push_back("LaserBullet");

	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(playerWeapons.begin(), playerWeapons.end(), gen);

	size_t count = std::min<size_t>(3, playerWeapons.size());
	std::vector<std::string> weapons(playerWeapons.begin(), playerWeapons.begin() + count);
	loadScene(weapons);
}

void Scene_NewWeapon::loadScene(const std::vector<std::string>& weapons)
{
	m_entityManager = EntityManager();

	auto title = m_entityManager.addEntity("ui", "Choose New Weapon");
	auto& tAnimation = title->add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true).animation;
	tAnimation.m_sprite.setScale(sf::Vector2f(2.f, 0.6f));
	title->add<CTransform>(Vec2f(width() / 2, height() * 0.10f));

	for (size_t i = 0; i < weapons.size(); i++)
	{
		auto button = m_entityManager.addEntity("button", weapons[i]);
		button->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
		auto& firstTransform = button->add<CTransform>(Vec2f(width() / 2, height() * (0.35f + 0.25*i)));
		button->add<CState>("unselected");
	}
}

void Scene_NewWeapon::update()
{
	m_entityManager.update();
	sHover();
	sAnimation();
}

void Scene_NewWeapon::sHover()
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

void Scene_NewWeapon::sAnimation()
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

void Scene_NewWeapon::onEnd()
{
	m_game->quit();
}

void Scene_NewWeapon::onExitScene()
{
	m_selectedIndex = 0;
	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.stop();

	auto& window = m_game->window();
	window.setView(window.getDefaultView());
}

void Scene_NewWeapon::onEnterScene()
{
	auto& window = m_game->window();
	window.setView(m_menuView);

	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.play();
}

void Scene_NewWeapon::select()
{
	for (auto& button : m_entityManager.getEntities("button"))
	{
		if (!Utils::IsInside(m_mousePos, button)) continue;
		if (!m_game->changeScene("PLAY", nullptr)) continue;

		if (button->name() == "MeeleSlash")
			m_player->add<CBasicAttack>();
		else if (button->name() == "RangedSlash")
			m_player->add<CSpecialAttack>();
		else if (button->name() == "FireRing")
			m_player->add<CRingAttack>();
		else if (button->name() == "Whirlpool")
			m_player->add<CWhirlAttack>();
		else if (button->name() == "Explosion")
			m_player->add<CExplodeAttack>();
		else if (button->name() == "LaserBullet")
			m_player->add<CBulletAttack>();

		onExitScene();
	}
}

void Scene_NewWeapon::sDoAction(const Action& action)
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

void Scene_NewWeapon::sRender()
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
		buttonText.setCharacterSize(200 * transform.scale);
		buttonText.setString(entity->name());
		buttonText.setOutlineColor(sf::Color(28, 30, 38));
		buttonText.setOutlineThickness(2.0f * transform.scale);
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
		buttonText.setCharacterSize(180);
		buttonText.setString(weaponData.name);
		buttonText.setOutlineColor(sf::Color(28, 30, 38));
		buttonText.setOutlineThickness(2.0f);
		auto bounds = buttonText.getLocalBounds();
		buttonText.setOrigin(bounds.position + bounds.size / 2.f);
		buttonText.setPosition(sf::Vector2f(
			transform.pos.x + buttonBounds.size.x / 10, transform.pos.y - buttonBounds.size.y / 5));
		window.draw(buttonText);

		auto descriptText = sf::Text(m_game->assets().getFont("ByteBounce"));
		descriptText.setCharacterSize(100);
		descriptText.setString(weaponData.description);
		descriptText.setOutlineColor(sf::Color(28, 30, 38));
		descriptText.setOutlineThickness(2.0f);
		bounds = descriptText.getLocalBounds();
		descriptText.setOrigin(bounds.position + bounds.size / 2.f);
		descriptText.setPosition(sf::Vector2f(
			transform.pos.x + buttonBounds.size.x / 10, transform.pos.y + buttonBounds.size.y / 5));
		window.draw(descriptText);
	}
}