#include "Scene_NewWeapon.h"
#include "Scene_Play.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "Utils.hpp"

#include <iostream>

Scene_NewWeapon::Scene_NewWeapon(GameEngine* gameEngine, std::shared_ptr<Entity> player)
	: Scene(gameEngine)
{
	init();
}

void Scene_NewWeapon::init()
{
	/*registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");
	registerAction(sf::Keyboard::Scan::D, "PLAY");
	registerAction(sf::Keyboard::Scan::Escape, "QUIT");*/

	m_musicName = "Awakened";
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

	loadScene();
}

void Scene_NewWeapon::loadScene()
{
	m_entityManager = EntityManager();

	auto title = m_entityManager.addEntity("ui", "Choose New Weapon");
	auto& tAnimation = title->add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true).animation;
	tAnimation.m_sprite.setScale(sf::Vector2f(2.f, 0.6f));
	title->add<CTransform>(Vec2f(width() / 2, height() * 0.10f));

	auto firstButton = m_entityManager.addEntity("button", "Option 1");
	firstButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& firstTransform = firstButton->add<CTransform>(Vec2f(width() / 2, height() * 0.35f));
	firstButton->add<CState>("unselected");

	auto secondButton = m_entityManager.addEntity("button", "Option 2");
	secondButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& secondTransform = secondButton->add<CTransform>(Vec2f(width() / 2, height() * 0.6f));
	secondButton->add<CState>("unselected");

	auto thirdButton = m_entityManager.addEntity("button", "Option 3");
	thirdButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& thirdTransform = thirdButton->add<CTransform>(Vec2f(width() / 2, height() * 0.85f));
	thirdButton->add<CState>("unselected");
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

		if (button->name() == "Option 1" && m_game->changeScene("PLAY", nullptr))
			onExitScene();
		else if (button->name() == "Option 2" && m_game->changeScene("PLAY", nullptr))
			onExitScene();
		else if (button->name() == "Option 3" && m_game->changeScene("PLAY", nullptr))
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


	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CAnimation>()) continue;

		auto& animation = entity->get<CAnimation>().animation;
		auto& transform = entity->get<CTransform>();

		animation.m_sprite.setPosition(transform.pos);
		if (entity->tag() == "button")
			animation.m_sprite.setScale({ transform.scale * 1.8f, transform.scale * 0.95f });
		window.draw(animation.m_sprite);

		auto buttonText = sf::Text(m_game->assets().getFont("FutureMillennium"));
		buttonText.setCharacterSize(200 * transform.scale);
		buttonText.setString(entity->name());
		buttonText.setOutlineColor(sf::Color(204, 226, 225));
		buttonText.setOutlineThickness(5.0f * transform.scale);
		auto bounds = buttonText.getLocalBounds();
		buttonText.setOrigin(bounds.position + bounds.size / 2.f);
		buttonText.setPosition(transform.pos);
		window.draw(buttonText);
	}
}