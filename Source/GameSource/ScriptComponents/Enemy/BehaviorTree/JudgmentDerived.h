#pragma once
#include "JudgmentBase.h"
#include "../EnemyCom.h"

// BattleNode�ɑJ�ڂł��邩����
class BattleJudgment : public JudgmentBase
{
public:
	BattleJudgment(EnemyCom* enemy) :JudgmentBase(enemy) {};
	// ����
	bool Judgment();
};

// AttackNode�ɑJ�ڂł��邩����
class AttackJudgment : public JudgmentBase
{
public:
	AttackJudgment(EnemyCom* enemy) :JudgmentBase(enemy) {};
	// ����
	bool Judgment();
};

// WanderNode�ɑJ�ڂł��邩����
class WanderJudgment : public JudgmentBase
{
public:
	WanderJudgment(EnemyCom* enemy) :JudgmentBase(enemy) {};
	// ����
	bool Judgment();
};
