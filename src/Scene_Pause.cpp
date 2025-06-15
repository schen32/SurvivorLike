#include "Scene_Pause.h"
#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"
#include "Utils.hpp"

#include <iostream>

Scene_Pause::Scene_Pause(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

void Scene_Pause::init()
{
	/*registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");
	registerAction(sf::Keyboard::Scan::D, "PLAY");
	registerAction(sf::Keyboard::Scan::Escape, "QUIT");*/

	m_musicName = "M.S.Solar";
	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.setVolume(20);
	bgm.setLooping(true);
	bgm.play();

	auto& window = m_game->window();
	sf::Vector2f windowSize = sf::Vector2f(width(), height());
	m_pauseView.setCenter(windowSize / 2.0f);
	m_pauseView.setSize(windowSize);
	m_pauseView.setViewport(sf::FloatRect({0.25f, 0.25f}, {0.5f, 0.5f}));
	window.setView(m_pauseView);

	loadScene();
}

void Scene_Pause::loadScene()
{
	m_entityManager = EntityManager();

	auto title = m_entityManager.addEntity("ui", "Paused");
	auto& tAnimation = title->add<CAnimation>(m_game->assets().getAnimation("ButtonHover"), true).animation;
	tAnimation.m_sprite.setScale(sf::Vector2f(1.2f, 0.8f));
	title->add<CTransform>(Vec2f(width() / 2, height() * 0.2f));

	auto playButton = m_entityManager.addEntity("button", "Resume");
	playButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& pbTransform = playButton->add<CTransform>(Vec2f(width() / 2, height() * 0.4f));
	pbTransform.scale = 0.5f;
	playButton->add<CState>("unselected");

	auto continueButton = m_entityManager.addEntity("button", "Quit");
	continueButton->add<CAnimation>(m_game->assets().getAnimation("Button"), true);
	auto& cTransform = continueButton->add<CTransform>(Vec2f(width() / 2, height() * 0.55f));
	cTransform.scale = 0.5f;
	continueButton->add<CState>("unselected");
}

void Scene_Pause::update()
{
	m_entityManager.update();
	sHover();
	sAnimation();
}

void Scene_Pause::sHover()
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

void Scene_Pause::sAnimation()
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

void Scene_Pause::onEnd()
{
	m_game->quit();
}

void Scene_Pause::onExitScene()
{
	m_selectedIndex = 0;
	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.stop();
}

void Scene_Pause::onEnterScene()
{
	auto& window = m_game->window();
	window.setView(m_pauseView);

	auto& bgm = m_game->assets().getMusic(m_musicName);
	bgm.play();
}

void Scene_Pause::select()
{
	for (auto& button : m_entityManager.getEntities("button"))
	{
		if (!Utils::IsInside(m_mousePos, button)) continue;

		if (button->name() == "Resume" && m_game->changeScene("PLAY", nullptr))
			onExitScene();
		else if (button->name() == "Quit" && m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game)))
			onExitScene();
	}
}

void Scene_Pause::sDoAction(const Action& action)
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

void Scene_Pause::sRender()
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
			animation.m_sprite.setScale({ transform.scale, transform.scale });
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