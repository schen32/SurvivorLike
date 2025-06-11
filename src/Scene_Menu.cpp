#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "Utils.hpp"

#include <iostream>

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

void Scene_Menu::init()
{
	registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");
	registerAction(sf::Keyboard::Scan::D, "PLAY");
	registerAction(sf::Keyboard::Scan::Escape, "QUIT");

    m_musicName = "Awakened";
    auto& bgm = m_game->assets().getMusic(m_musicName);
    bgm.setVolume(20);
    bgm.setLooping(true);
    bgm.play();

	loadMenu();
}

void Scene_Menu::loadMenu()
{
	m_entityManager = EntityManager();

	auto title = m_entityManager.addEntity("ui", "Alien Survivors");
	auto& tAnimation = title->add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true).animation;
	tAnimation.m_sprite.setScale(sf::Vector2f(2.f, 0.8f));
	title->add<CTransform>(Vec2f(width() / 2, height() * 0.2f));

	auto playButton = m_entityManager.addEntity("button", "Play");
	playButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& pbTransform = playButton->add<CTransform>(Vec2f(width() / 2, height() * 0.4f));
	pbTransform.scale = 0.5f;
	playButton->add<CState>("unselected");

	auto continueButton = m_entityManager.addEntity("button", "Continue");
	continueButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& cTransform = continueButton->add<CTransform>(Vec2f(width() / 2, height() * 0.55f));
	cTransform.scale = 0.5f;
	continueButton->add<CState>("unselected");

	auto quitButton = m_entityManager.addEntity("button", "Quit");
	quitButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& qTransform = quitButton->add<CTransform>(Vec2f(width() / 2, height() * 0.70f));
	qTransform.scale = 0.5f;
	quitButton->add<CState>("unselected");
}

void Scene_Menu::update()
{
	m_entityManager.update();
	sHover();
	sAnimation();
}

void Scene_Menu::sHover()
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

void Scene_Menu::sAnimation()
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

void Scene_Menu::onEnd()
{
	m_game->quit();
}

void Scene_Menu::onPause()
{
    m_selectedMenuIndex = 0;
    auto& bgm = m_game->assets().getMusic(m_musicName);
    bgm.stop();
}

void Scene_Menu::onResume()
{
    auto& bgm = m_game->assets().getMusic(m_musicName);
    bgm.play();
}

void Scene_Menu::select()
{
	for (auto& button : m_entityManager.getEntities("button"))
	{
		if (!Utils::IsInside(m_mousePos, button)) continue;

		if (button->name() == "Play" &&
			m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game)))
			onPause();
		else if (button->name() == "Continue" &&
			m_game->changeScene("PLAY", nullptr))
			onPause();
		else if (button->name() == "Quit")
			onEnd();
			
	}
}

void Scene_Menu::sDoAction(const Action& action)
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

void Scene_Menu::sRender()
{
    auto& window = m_game->window();
	window.clear(sf::Color(204, 226, 225));
    
	for (auto& entity : m_entityManager.getEntities())
	{
		if (!entity->has<CAnimation>()) continue;

		auto& animation = entity->get<CAnimation>().animation;
		auto& transform = entity->get<CTransform>();

		animation.m_sprite.setPosition(transform.pos);
		if (entity->tag() == "button")
			animation.m_sprite.setScale({ transform.scale, transform.scale });
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