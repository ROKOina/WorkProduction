#pragma once
#include "ActionBase.h"
#include "../EnemyCom.h"

// ë“ã@çsìÆ
class IdleAction : public ActionBase
{
public:
	IdleAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// úpújçsìÆ
class WanderAction : public ActionBase
{
public:
	WanderAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// í«ê’çsìÆ
class PursuitAction : public ActionBase
{
public:
	PursuitAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// í èÌçUåÇ
class AttackAction : public ActionBase
{
public:
	AttackAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};
