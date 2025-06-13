#pragma once

#include "Scene.h"
#include <unordered_map>
#include <memory>
#include <deque>

#include "EntityManager.hpp"

class Scene_NewWeapon : public Scene
{
protected:
	size_t m_selectedIndex = 0;
	sf::View m_menuView;
	std::string m_musicName;
	Vec2f m_mousePos;

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
	Scene_NewWeapon() = default;
	Scene_NewWeapon(GameEngine* gameEngine = nullptr, std::shared_ptr<Entity> player = nullptr);
	void sRender();
};