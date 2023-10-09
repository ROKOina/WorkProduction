#pragma once

class EnemyCom;

enum Action
{
	End_STEP = 99,	//�����I��
};

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

	//�r���Ŏ��s���I�����鎞
	void EndActionStep()
	{
		step_ = static_cast<int>(Action::End_STEP);
	}

	void StartActionStep()
	{
		step_ = 0;
	}

protected:
	EnemyCom* owner_;
	int step_ = 0;
	float runTimer_ = 0;
};
