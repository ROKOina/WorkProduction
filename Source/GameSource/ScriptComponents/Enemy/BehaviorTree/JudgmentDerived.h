#pragma once
#include "JudgmentBase.h"
#include "../EnemyCom.h"

// BattleNodeに遷移できるか判定
class BattleJudgment : public JudgmentBase
{
public:
	BattleJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// 判定
	bool Judgment();
};

// NearAttackNodeに遷移できるか判定
class NearAttackJudgment : public JudgmentBase
{
public:
	NearAttackJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// 判定
	bool Judgment();
};

// WanderNodeに遷移できるか判定
class WanderJudgment : public JudgmentBase
{
public:
	WanderJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// 判定
	bool Judgment();
};
