#pragma once

class EnemyCom;

// �s���������N���X
class ActionBase
{
public:
	ActionBase(EnemyCom* enemy) :owner_(enemy) {}
	// ���s���
	enum class State
	{
		Run,		// ���s��
		Failed,		// ���s���s
		Complete,	// ���s����
	};

	// ���s����(�������z�֐�)
	virtual ActionBase::State Run(float elapsedTime) = 0;
protected:
	EnemyCom* owner_;
	int step_ = 0;
	float runTimer_ = 0;
};
