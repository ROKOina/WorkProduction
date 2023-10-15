#pragma once
#include "JudgmentBase.h"
#include "../EnemyCom.h"

// BattleNode�ɑJ�ڂł��邩����
class BattleJudgment : public JudgmentBase
{
public:
	BattleJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ����
	bool Judgment();
};

// NearAttackNode�ɑJ�ڂł��邩����
class NearAttackJudgment : public JudgmentBase
{
public:
	NearAttackJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ����
	bool Judgment();
};

// WanderNode�ɑJ�ڂł��邩����
class WanderJudgment : public JudgmentBase
{
public:
	WanderJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ����
	bool Judgment();
};
