#include "Scene_Menu.h"
#include "Scene_Option.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Entity.hpp"
#include "Action.hpp"
#include "Utils.hpp"

#include <iostream>

Scene_Option::Scene_Option(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

void Scene_Option::init()
{
	loadMenu();
}

void Scene_Option::loadMenu()
{
	m_entityManager = EntityManager();

	auto title = m_entityManager.addEntity("ui", "Options");
	auto& tAnimation = title->add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true).animation;
	tAnimation.m_sprite.setScale(sf::Vector2f(1.6f, 0.8f));
	title->add<CTransform>(Vec2f(width() / 2, height() * 0.2f));

	auto fullButton = m_entityManager.addEntity("button", "Windowed");
	fullButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& fTransform = fullButton->add<CTransform>(Vec2f(width() / 2, height() * 0.4f));
	fTransform.scale = 0.5f;
	fullButton->add<CState>("unselected");

	auto backButton = m_entityManager.addEntity("button", "Back");
	backButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& bTransform = backButton->add<CTransform>(Vec2f(width() / 2, height() * 0.55f));
	bTransform.scale = 0.5f;
	backButton->add<CState>("unselected");
}

void Scene_Option::update()
{
	m_entityManager.update();
	sHover();
	sAnimation();
}

void Scene_Option::sHover()
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

void Scene_Option::sAnimation()
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

void Scene_Option::onEnd()
{
	m_game->quit();
}

void Scene_Option::onExitScene()
{

}

void Scene_Option::onEnterScene()
{

}

void Scene_Option::select()
{
	for (auto& button : m_entityManager.getEntities("button"))
	{
		if (!Utils::IsInside(m_mousePos, button)) continue;

		if (button->name() == "Fullscreen")
		{
			auto& m_window = m_game->window();
			// Close and recreate the window in fullscreen
			m_window.close();
			auto desktop = sf::VideoMode::getDesktopMode();
			m_window.create(desktop, "Alien Survivor", sf::Style::None);
			m_game->m_isFullscreen = true;
			m_window.setFramerateLimit(60);

			button->m_name = "Windowed";
		}
		else if (button->name() == "Windowed")
		{
			auto& m_window = m_game->window();
			// Close and recreate the window in fullscreen
			m_window.close();
			auto desktop = sf::VideoMode::getDesktopMode();
			m_window.create(desktop, "Alien Survivor", sf::Style::Default);
			m_game->m_isFullscreen = false;
			m_window.setFramerateLimit(60);

			button->m_name = "Fullscreen";
		}
		else if (button->name() == "Back" &&
			m_game->changeScene("MENU", nullptr))
			onExitScene();
	}
}

void Scene_Option::sDoAction(const Action& action)
{
	if (action.m_type == "START")
	{
		/*if (action.m_name == "PLAY")
		{
			if (m_selectedMenuIndex == 0)
			{
				if (m_game->changeScene("PLAY",
					std::make_shared<Scene_Play>(m_game, m_levelPaths[m_selectedMenuIndex])))
					onExitScene();
			}
			else if (m_selectedMenuIndex == 1)
			{
				if (m_game->changeScene("PLAY", nullptr))
					onExitScene();
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

void Scene_Option::sRender()
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
			animation.m_sprite.setScale({ transform.scale * 1.2f, transform.scale });
		window.draw(animation.m_sprite);

		auto buttonText = sf::Text(m_game->assets().getFont("FutureMillennium"));
		buttonText.setCharacterSize(200 * transform.scale);
		buttonText.setString(entity->name());
		buttonText.setOutlineThickness(2.0f * transform.scale);
		buttonText.setOutlineColor(sf::Color(86, 106, 137));
		auto bounds = buttonText.getLocalBounds();
		buttonText.setOrigin(bounds.position + bounds.size / 2.f);
		buttonText.setPosition(transform.pos);
		window.draw(buttonText);
	}
}