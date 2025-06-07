#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"

#include <iostream>

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

void Scene_Menu::init()
{
	m_title = "Cyber Runner";
	m_menuStrings.push_back("Start");
	m_menuStrings.push_back("Options");

	m_levelPaths.push_back("assets/play.txt");

	m_menuText.setFont(m_game->assets().getFont("FutureMillennium"));

	registerAction(sf::Keyboard::Scan::W, "UP");
	registerAction(sf::Keyboard::Scan::S, "DOWN");
	registerAction(sf::Keyboard::Scan::D, "PLAY");
	registerAction(sf::Keyboard::Scan::Escape, "QUIT");
}

void Scene_Menu::update()
{
	
}

void Scene_Menu::onEnd()
{
	m_game->quit();
}

void Scene_Menu::sDoAction(const Action& action)
{
	if (action.m_type == "START")
	{
		if (action.m_name == "PLAY")
		{
			m_game->changeScene("PLAY",
				std::make_shared<Scene_Play>(m_game, m_levelPaths[m_selectedMenuIndex]));
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
		}
		else if (action.m_name == "QUIT")
		{
            onEnd();
		}
	}
}

void Scene_Menu::sRender()
{
    auto& window = m_game->window();
    window.clear(sf::Color(20, 22, 26));

    // Draw planet
    sf::CircleShape planet(400.0f);
    planet.setPointCount(150);
    planet.setFillColor(sf::Color(158, 97, 22));
    planet.setOrigin({ planet.getRadius(), planet.getRadius() });  // Fixed origin calculation
    planet.setPosition({ width() * 0.25f, height() * 0.5f });     // Simplified position calculation
    window.draw(planet);

    // Draw title
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color::White);
    m_menuText.setCharacterSize(100);

    // Center title horizontally
    float titleWidth = m_menuText.getLocalBounds().size.x;
    m_menuText.setPosition({ (width() - titleWidth) * 0.5f, 10 });
    window.draw(m_menuText);

    // Draw menu items
    m_menuText.setCharacterSize(40);
    m_menuTextBackground.setSize({ 200, 75 });
    m_menuTextBackground.setFillColor(sf::Color(158, 97, 22));

    float menuStartX = width() - 250.0f;
    float menuStartY = height() * 0.25f;  // More readable than (height() / 2.0f) / 2.0f

    for (size_t sI = 0; sI < m_menuStrings.size(); sI++)  // Changed to size_t
    {
        m_menuTextBackground.setPosition({ menuStartX, menuStartY + sI * 100.0f });

        // Calculate text position relative to background
        sf::FloatRect bgBounds = m_menuTextBackground.getLocalBounds();
        sf::FloatRect textBounds = m_menuText.getLocalBounds();

        float textPosX = m_menuTextBackground.getPosition().x +
            (bgBounds.size.x - textBounds.size.x) * 0.5f -
            textBounds.position.x;

        float textPosY = m_menuTextBackground.getPosition().y +
            (bgBounds.size.y - textBounds.size.y) * 0.5f -
            textBounds.position.y;

        if (m_selectedMenuIndex == sI)
        {
            m_menuText.setFillColor(sf::Color::Black);
            m_menuTextBackground.setOutlineColor(sf::Color::Red);
            m_menuTextBackground.setOutlineThickness(3);
        }
        else
        {
            m_menuText.setFillColor(sf::Color::White);
            m_menuTextBackground.setOutlineColor(sf::Color(150, 150, 150));
            m_menuTextBackground.setOutlineThickness(1);
        }

        m_menuText.setString(m_menuStrings[sI]);
        m_menuText.setPosition({ textPosX, textPosY });

        window.draw(m_menuTextBackground);
        window.draw(m_menuText);
    }

    // Draw controls help text
    const sf::Font& font = m_game->assets().getFont("FutureMillennium");  // Use reference to avoid copy
    sf::Text upText(font, "UP: W", 25);
    sf::Text downText(font, "DOWN: S", 25);
    sf::Text playText(font, "PLAY: D", 25);
    sf::Text backText(font, "QUIT: ESC", 25);

    float controlsY = static_cast<float>(window.getSize().y) - 100.0f;
    float padding = 40.0f;

    upText.setPosition({ 10.0f, controlsY });
    downText.setPosition({ upText.getPosition().x + upText.getLocalBounds().size.x + padding, controlsY });
    playText.setPosition({ downText.getPosition().x + downText.getLocalBounds().size.x + padding, controlsY });
    backText.setPosition({ playText.getPosition().x + playText.getLocalBounds().size.x + padding, controlsY });

    window.draw(upText);
    window.draw(downText);
    window.draw(playText);
    window.draw(backText);
}