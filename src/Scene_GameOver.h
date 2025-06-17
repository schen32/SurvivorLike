#pragma once

#include "Scene.h"
#include <unordered_map>
#include <memory>
#include <deque>

#include "EntityManager.hpp"

class Scene_GameOver : public Scene
{
protected:
	size_t m_selectedIndex = 0;
	sf::View m_pauseView;
	std::string m_musicName;
	Vec2f m_mousePos;
	std::shared_ptr<Entity> m_player;

	void init();
	void loadScene();
	void update();
	void onEnd();
	void sDoAction(const Action& action);
	void onEnterScene();
	void onExitScene();

	void select();
	void sHover();
	void sAnimation();
public:
	Scene_GameOver() = default;
	Scene_GameOver(GameEngine* gameEngine = nullptr, std::shared_ptr<Entity> player = nullptr);
	void sRender();
};
