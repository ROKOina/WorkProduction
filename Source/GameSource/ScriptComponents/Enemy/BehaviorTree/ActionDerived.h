#pragma once
#include "ActionBase.h"
#include "../EnemyCom.h"

// �ҋ@�s��
class IdleAction : public ActionBase
{
public:
	IdleAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �p�j�s��
class WanderAction : public ActionBase
{
public:
	WanderAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ǐՍs��
class PursuitAction : public ActionBase
{
public:
	PursuitAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ߐڒʏ�U��
class NearAttackAction : public ActionBase
{
public:
	NearAttackAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);

private:
	float timer_ = 0;
};
