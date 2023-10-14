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

// AttackNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class AttackJudgment : public JudgmentBase
{
public:
	AttackJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
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
