#pragma once
#include "ActionBase.h"
#include "../EnemyCom.h"

// �ҋ@�s��
class IdleAction : public ActionBase
{
public:
	IdleAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �p�j�s��
class WanderAction : public ActionBase
{
public:
	WanderAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ǐՍs��
class PursuitAction : public ActionBase
{
public:
	PursuitAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �ʏ�U��
class AttackAction : public ActionBase
{
public:
	AttackAction(EnemyCom* enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};
