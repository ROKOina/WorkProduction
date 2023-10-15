#pragma once
#include "ActionBase.h"
#include "../EnemyCom.h"

// ë“ã@çsìÆ
class IdleAction : public ActionBase
{
public:
	IdleAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// úpújçsìÆ
class WanderAction : public ActionBase
{
public:
	WanderAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// í«ê’çsìÆ
class PursuitAction : public ActionBase
{
public:
	PursuitAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// ãﬂê⁄í èÌçUåÇ
class NearAttackAction : public ActionBase
{
public:
	NearAttackAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);

private:
	float timer_ = 0;
};
