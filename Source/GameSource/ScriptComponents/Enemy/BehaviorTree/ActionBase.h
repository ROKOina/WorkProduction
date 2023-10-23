#pragma once

#include <memory>

class EnemyCom;

enum Action
{
	END_STEP = 99,	//強制終了
};

// 行動処理基底クラス
class ActionBase
{
public:
	ActionBase(std::shared_ptr<EnemyCom> enemy) :owner_(enemy) {}
	// 実行情報
	enum class State
	{
		Run,		// 実行中
		Failed,		// 実行失敗
		Complete,	// 実行成功
	};

	// 実行処理(純粋仮想関数)
	virtual ActionBase::State Run(float elapsedTime) = 0;

	//途中で実行を終了する時
	void EndActionStep()
	{
		step_ = static_cast<int>(Action::END_STEP);
	}

	void StartActionStep()
	{
		step_ = 0;
	}

protected:
	std::weak_ptr<EnemyCom> owner_;
	//EnemyCom* owner_;
	int step_ = 0;
	float runTimer_ = 0;
};
