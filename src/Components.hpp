#pragma once

#include "Animation.hpp"
#include "Assets.hpp"
#include "Vec2.hpp"
#include "Entity.hpp"

class Entity;

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
	float accel = 0;

	CTransform() = default;
	CTransform(const Vec2f& p)
		: pos(p) {}
	CTransform(const Vec2f& p, const Vec2f& v, float a)
		: pos(p), velocity(v), angle(a) {}
};

class CMoveAtSameVelocity : public Component
{
public:
	std::shared_ptr<Entity> target = nullptr;

	CMoveAtSameVelocity() = default;
	CMoveAtSameVelocity(std::shared_ptr<Entity> t) : target(t) {}
};

class CFollow : public Component
{
public:
	std::shared_ptr<Entity> target = nullptr;
	float steering_scale = 0.1f;

	CFollow() = default;
	CFollow(std::shared_ptr<Entity> t) : target(t) {}
};

class CHealth : public Component
{
public:
	int health = 0;

	CHealth() = default;
	CHealth(int h) : health(h) {}
};

class CDamage : public Component
{
public:
	int damage = 0;

	CDamage() = default;
	CDamage(int d) : damage(d) {}
};

class CLifespan : public Component
{
public:
	int lifespan = 0;
	int frameCreated = 0;

	CLifespan() = default;
	CLifespan(int lifespan, int frame)
		: lifespan(lifespan), frameCreated(frame) {}
};

class CInput : public Component
{
public:
	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool displayHitbox = false;
	bool autoAttack = false;
	bool basicAttack = false;
	bool specialAttack = false;

	CInput() = default;
};

class CBasicAttack : public Component
{
public:
	int cooldown = 30;
	int lastAttackTime = 0;
	float scale = 1.0f;
	int duration = 15;
	int pierce = 5;
	int distanceFromPlayer = 30;
	int damage = 1;

	CBasicAttack() = default;
	CBasicAttack(int lat)
		: lastAttackTime(lat) { }
};

class CSpecialAttack : public Component
{
public:
	int cooldown = 90;
	int lastAttackTime = 0;
	float scale = 1.0f;
	int duration = 60;
	int pierce = 10;
	int speed = 10;
	float decel = -0.2f;
	int damage = 1;

	CSpecialAttack() = default;
	CSpecialAttack(int lat)
		: lastAttackTime(lat) { }
};

class CKnockback : public Component
{
public:
	Vec2f direction = { 0, 0 };
	float magnitude = 0;
	int duration = 0;
	float decel = 0;

	CKnockback() = default;
	CKnockback(const Vec2f& dir, float m, int d, float deceleration)
		: direction(dir), magnitude(m), duration(d), decel(deceleration) { }
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
	std::string state = "idle";

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