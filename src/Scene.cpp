#include "Scene.h"
#include "GameEngine.h"

Scene::Scene(GameEngine* gameEngine)
	: m_game(gameEngine) { }

void Scene::setPaused(bool paused)
{
	m_paused = paused;
}

size_t Scene::width() const
{
	return m_game->window().getSize().x;
}

size_t Scene::height() const
{
	return m_game->window().getSize().y;
}

size_t Scene::currentFrame() const
{
	return m_currentFrame;
}

const ActionMap& Scene::getActionMap() const
{
	return m_actionMap;
}

void Scene::registerAction(sf::Keyboard::Scan keyCode, const std::string& name)
{
	m_actionMap[keyCode] = name;
}

bool Scene::hasEnded() const
{
	return m_hasEnded;
}

void Scene::simulate(const size_t frames)
{
	for (size_t i = 0; i < frames; i++)
	{
		update();
	}
}

void Scene::doAction(const Action& action)
{
	sDoAction(action);
}