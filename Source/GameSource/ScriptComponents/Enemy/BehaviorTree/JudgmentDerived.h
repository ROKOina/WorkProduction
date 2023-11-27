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

// BackMoveNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class BackMoveJudgment : public JudgmentBase
{
public:
	BackMoveJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
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

// RoutePathNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class RoutePathJudgment : public JudgmentBase
{
public:
	RoutePathJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ”»’è
	bool Judgment();
};

// AttackIdleNode‚É‘JˆÚ‚Å‚«‚é‚©”»’è
class AttackIdleJudgment : public JudgmentBase
{
public:
	AttackIdleJudgment(std::shared_ptr<EnemyCom> enemy) :JudgmentBase(enemy) {};
	// ”»’è
	bool Judgment();
};
