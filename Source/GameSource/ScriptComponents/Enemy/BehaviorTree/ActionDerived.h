#pragma once
#include "ActionBase.h"
#include "../EnemyCom.h"

// 待機行動
class IdleAction : public ActionBase
{
public:
	IdleAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 徘徊行動
class WanderAction : public ActionBase
{
public:
	WanderAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 追跡行動
class PursuitAction : public ActionBase
{
public:
	PursuitAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

//逃走行動
class BackMoveAction : public ActionBase
{
public:
	BackMoveAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// 攻撃出来ないときはプレイヤーを囲むように移動
class RoutePathAction : public ActionBase
{
public:
	RoutePathAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);

private:		
	//四方のどこに近くの敵がいるか見る
	bool quad_[4] = {};	//0:左上 1:左下 2:右上 3:右下

	//経路探査用変数
	float pathTimer_ = 1;
	std::vector<DirectX::XMFLOAT3> routePos_ = {};
};

// 近接通常攻撃
class AttackAction : public ActionBase
{
public:
	AttackAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);

private:
	float timer_ = 0;
};

// 戦闘待機行動
class AttackIdleAction : public ActionBase
{
public:
	AttackIdleAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

