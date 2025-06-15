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
	float scale = 1.0f;
	Vec2f velocity = { 0.0, 0.0 };
	float angle = 0;
	float accel = 0;

	CTransform() = default;
	CTransform(const Vec2f& p)
		: pos(p) {}
	CTransform(const Vec2f& p, const Vec2f& v)
		: pos(p), velocity(v) {}
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
	float speed = 1.0f;

	CFollow() = default;
	CFollow(std::shared_ptr<Entity> t, float s) : target(t), speed(s) {}
};

class CHealth : public Component
{
public:
	int health = 0;
	int maxHealth = 0;
	int lastTakenDamage = 0;
	int invulTime = 10;

	CHealth() = default;
	CHealth(int h) : health(h), maxHealth(h) {}
};

class CDamage : public Component
{
public:
	int damage = 0;

	CDamage() = default;
	CDamage(int d) : damage(d) {}
};

class CDisappearingText : public Component
{
public:
	sf::Font font;
	sf::Text text = sf::Text(font);
	Vec2f velocity = Vec2f(0, -1.0f);
	int lifetime = 60;

	CDisappearingText() = default;
	CDisappearingText(const sf::Text& t)
		: text(t) { }
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
	bool autoAim = true;
	bool autoAttack = true;
	bool basicAttack = false;
	bool specialAttack = false;
	bool bulletAttack = false;
	bool ringAttack = false;
	bool explodeAttack = false;
	bool whirlAttack = false;

	CInput() = default;
};

class CBasicAttack : public Component
{
public:
	int cooldown = 50;
	int lastAttackTime = 0;
	float scale = 1.0f;
	int duration = 15;
	int health = 100;
	int distanceFromPlayer = 30;
	int damage = 10;
	float knockMagnitude = 5.f;
	int knockDuration = 30;
	int level = 1;

	CBasicAttack() = default;
	CBasicAttack(int lat)
		: lastAttackTime(lat) { }
};

class CSpecialAttack : public Component
{
public:
	int cooldown = 150;
	int lastAttackTime = 0;
	float scale = 1.0f;
	int duration = 60;
	int health = 150;
	int speed = 10;
	float decel = -0.2f;
	int damage = 15;
	float knockMagnitude = 5.f;
	int knockDuration = 30;
	int level = 1;

	CSpecialAttack() = default;
	CSpecialAttack(int lat)
		: lastAttackTime(lat) { }
};

class CBulletAttack: public Component
{
public:
	int cooldown = 40;
	int lastAttackTime = 0;
	float scale = 2.0f;
	int duration = 90;
	int health = 50;
	int speed = 10;
	float decel = 0.f;
	int damage = 10;
	float knockMagnitude = 5.f;
	int knockDuration = 30;
	int level = 1;

	CBulletAttack() = default;
	CBulletAttack(int lat)
		: lastAttackTime(lat) {
	}
};

class CRingAttack : public Component
{
public:
	int cooldown = 500;
	int lastAttackTime = 0;
	float scale = 2.0f;
	int duration = 300;
	int health = 1000;
	int damage = 10;
	float knockMagnitude = 5.f;
	int knockDuration = 30;
	int level = 1;

	CRingAttack() = default;
	CRingAttack(int lat)
		: lastAttackTime(lat) {
	}
};

class CExplodeAttack : public Component
{
public:
	int cooldown = 100;
	int lastAttackTime = 0;
	float scale = 2.0f;
	int duration = 25;
	int health = 100;
	int damage = 20;
	float knockMagnitude = 15.f;
	int knockDuration = 50;
	int level = 1;

	CExplodeAttack() = default;
	CExplodeAttack(int lat)
		: lastAttackTime(lat) {
	}
};

class CWhirlAttack : public Component
{
public:
	int cooldown = 400;
	int lastAttackTime = 0;
	float scale = 2.0f;
	int duration = 300;
	int health = 1000;
	int damage = 5;
	float attractStrength = 50.0f;
	float attractRadius = 150.0f;
	float knockMagnitude = 0.f;
	float knockDuration = 30;
	int level = 1;

	CWhirlAttack() = default;
	CWhirlAttack(int lat)
		: lastAttackTime(lat) {
	}
};

class CAttractor : public Component
{
public:
	float strength = 0;
	float radius = 0;

	CAttractor() = default;
	CAttractor(float s, float r) : strength(s), radius(r) {}
};

class CKnockback : public Component
{
public:
	float magnitude = 0;
	int duration = 0;
	bool beingKnockedback = false;

	CKnockback() = default;
	CKnockback(float m, int d)
		: magnitude(m), duration(d) { }
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
	int prevScoreThreshold = 0;
	int nextScoreThreshold = 100;
	int level = 0;

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