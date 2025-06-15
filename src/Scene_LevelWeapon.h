#pragma once

#include "Scene.h"
#include <unordered_map>
#include <memory>
#include <deque>

#include "EntityManager.hpp"

class Scene_LevelWeapon : public Scene
{
protected:
	struct WeaponData
	{
		Animation animation;
		std::string name;
		std::string description;
		int level = 0;
	};

	size_t m_selectedIndex = 0;
	sf::View m_menuView;
	std::string m_musicName;
	Vec2f m_mousePos;
	std::unordered_map<std::string, WeaponData>  m_weaponMap;
	std::shared_ptr<Entity> m_player;

	void init(std::shared_ptr<Entity> player);
	void loadScene(const std::vector<std::string>& weapons);
	void update();
	void onEnd();
	void sDoAction(const Action& action);
	void onEnterScene();
	void onExitScene();

	void select();
	void sHover();
	void sAnimation();
public:
	Scene_LevelWeapon() = default;
	Scene_LevelWeapon(GameEngine* gameEngine = nullptr, std::shared_ptr<Entity> player = nullptr);
	void sRender();
};
