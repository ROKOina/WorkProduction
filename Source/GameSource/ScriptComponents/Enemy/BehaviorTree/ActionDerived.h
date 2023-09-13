#pragma once
#include "ActionBase.h"
#include "../EnemyCom.h"

// 待機行動
class IdleAction : public ActionBase
{
public:
	IdleAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 徘徊行動
class WanderAction : public ActionBase
{
public:
	WanderAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 追跡行動
class PursuitAction : public ActionBase
{
public:
	PursuitAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 通常攻撃
class AttackAction : public ActionBase
{
public:
	AttackAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);

private:
	float timer_ = 0;
};

// ダメージ
class DamageAction : public ActionBase
{
public:
	DamageAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};
