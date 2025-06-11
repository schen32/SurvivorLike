#pragma once

#include "Vec2.hpp"
#include "Entity.hpp"
#include "Components.hpp"

class Utils
{
public:
	Utils() = default;

	bool static IsInside(const Vec2f& pos, std::shared_ptr<Entity> entity)
	{
		auto& ePosition = entity->get<CTransform>().pos;
		auto& eSize = entity->get<CAnimation>().animation.m_size;

		if (ePosition.x - eSize.x / 2 <= pos.x && pos.x <= ePosition.x + eSize.x / 2 &&
			ePosition.y - eSize.y / 2 <= pos.y && pos.y <= ePosition.y + eSize.y / 2)
		{
			return true;
		}
		return false;
	}
};