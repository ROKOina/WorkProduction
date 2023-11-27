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

//�����s��
class BackMoveAction : public ActionBase
{
public:
	BackMoveAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

// �U���o���Ȃ��Ƃ��̓v���C���[���͂ނ悤�Ɉړ�
class RoutePathAction : public ActionBase
{
public:
	RoutePathAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);

private:		
	//�l���̂ǂ��ɋ߂��̓G�����邩����
	bool quad_[4] = {};	//0:���� 1:���� 2:�E�� 3:�E��

	//�o�H�T���p�ϐ�
	float pathTimer_ = 1;
	std::vector<DirectX::XMFLOAT3> routePos_ = {};
};

// �ߐڒʏ�U��
class AttackAction : public ActionBase
{
public:
	AttackAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);

private:
	float timer_ = 0;
};

// �퓬�ҋ@�s��
class AttackIdleAction : public ActionBase
{
public:
	AttackIdleAction(std::shared_ptr<EnemyCom> enemy) :ActionBase(enemy) {}
	ActionBase::State Run(float elapsedTime);
};

