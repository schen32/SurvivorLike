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

    m_musicName = "Gymnopedie";
    auto& bgm = m_game->assets().getMusic(m_musicName);
    bgm.setVolume(120);
    bgm.setLooping(true);
    bgm.play();

	loadMenu();
}

void Scene_Menu::loadMenu()
{
	m_entityManager = EntityManager();

	auto playButton = m_entityManager.addEntity("button", "playButton");
	playButton->add<CAnimation>(m_game->assets().getAnimation("PlayButton"), true);
	playButton->add<CTransform>(Vec2f(width(), height())/2);
	playButton->add<CState>("unselected");
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
		if (button->name() == "playButton")
		{
			if (buttonState == "selected" && buttonAnimation.m_name != "PlayButtonHover")
			{
				button->add<CAnimation>(m_game->assets().getAnimation("PlayButtonHover"), true);
				playSound("BubblierStep", 20);
			}
			else if (buttonState == "unselected" && buttonAnimation.m_name != "PlayButton")
			{
				button->add<CAnimation>(m_game->assets().getAnimation("PlayButton"), true);
				playSound("BubblierStep", 20);
			}
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

		if (button->name() == "playButton" &&
			m_game->changeScene("PLAY",std::make_shared<Scene_Play>(m_game)))
			onPause();
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
		window.draw(animation.m_sprite);
	}
}