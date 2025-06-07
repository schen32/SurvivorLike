#pragma once

#include "Animation.hpp"
#include "Assets.hpp"
#include "Vec2.hpp"

class Component
{
public:
	bool exists = false;
};

class CTransform : public Component
{
public:
	Vec2f pos = { 0.0, 0.0 };
	Vec2f prevPos = { 0.0, 0.0 };
	Vec2f scale = { 1.0, 1.0 };
	Vec2f velocity = { 0.0, 0.0 };
	float angle = 0;

	CTransform() = default;
	CTransform(const Vec2f& p)
		: pos(p) {}
	CTransform(const Vec2f& p, const Vec2f& v, float a)
		: pos(p), velocity(v), angle(a) {}
	CTransform(const Vec2f& p, const Vec2f& pP, const Vec2f s, const Vec2f& v, float a)
		: pos(p), prevPos(pP), scale(s), velocity(v), angle(a) {}
};

class CLifespan : public Component
{
public:
	int lifespan = 0;
	int frameCreated = 0;

	CLifespan() = default;
	CLifespan(int duration, int frame)
		: lifespan(duration), frameCreated(frame) {}
};

class CInput : public Component
{
public:
	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool shoot = false;
	bool canShoot = true;
	bool canJump = true;

	CInput() = default;
};

class CBoundingBox : public Component
{
public:
	Vec2f size;
	Vec2f halfSize;

	CBoundingBox() = default;
	CBoundingBox(const Vec2f& s)
		: size(s), halfSize(s / 2) { }
};

class CAnimation : public Component
{
public:
	Animation animation;
	bool repeat = false;

	CAnimation() = default;
	CAnimation(const Animation& a, bool r)
		: animation(a), repeat(r) {}
};

class CGravity : public Component
{
public:
	float gravity = 9.81;

	CGravity() = default;
	CGravity(float g) : gravity(g) {}
};

class CState : public Component
{
public:
	std::string state = "jumping";

	CState() = default;
	CState(const std::string& s) : state(s) {}
};

class CScore : public Component
{
public:
	int score = 0;

	CScore() = default;
	CScore(int s) : score(s) {}
};

class CDraggable : public Component
{
public:
	bool dragging = false;

	CDraggable() = default;
	CDraggable(bool draggable) : dragging(draggable) {}
};