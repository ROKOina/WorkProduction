#pragma once
#include "JudgmentBase.h"
#include "../EnemyCom.h"

// BattleNodeに遷移できるか判定
class BattleJudgment : public JudgmentBase
{
public:
	BattleJudgment(EnemyCom* enemy) :JudgmentBase(enemy) {};
	// 判定
	bool Judgment();
};

// AttackNodeに遷移できるか判定
class AttackJudgment : public JudgmentBase
{
public:
	AttackJudgment(EnemyCom* enemy) :JudgmentBase(enemy) {};
	// 判定
	bool Judgment();
};

// WanderNodeに遷移できるか判定
class WanderJudgment : public JudgmentBase
{
public:
	WanderJudgment(EnemyCom* enemy) :JudgmentBase(enemy) {};
	// 判定
	bool Judgment();
};
