#pragma once
#include "JudgmentBase.h"
#include "../EnemyCom.h"

// BattleNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class BattleJudgment : public JudgmentBase
{
public:
	BattleJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ”»’è
	bool Judgment();
};

// NearAttackNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class NearAttackJudgment : public JudgmentBase
{
public:
	NearAttackJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ”»’è
	bool Judgment();
};

// WanderNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class WanderJudgment : public JudgmentBase
{
public:
	WanderJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ”»’è
	bool Judgment();
};
