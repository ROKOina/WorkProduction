#include "BehaviorData.h"
#include "NodeBase.h"


// シーケンスノードのポップ
std::shared_ptr<NodeBase> BehaviorData::PopSequenceNode()
{
	// 空ならNULL
	if (sequenceStack_.empty() != 0)
	{
		return nullptr;
	}
	std::shared_ptr<NodeBase> node = sequenceStack_.top().lock();
	if (node != nullptr)
	{
		// 取り出したデータを削除
		sequenceStack_.pop();
	}
	return node;
}

// シーケンスステップのゲッター
int BehaviorData::GetSequenceStep(int id)
{
	if (runSequenceStepMap_.count(id) == 0)
	{
		runSequenceStepMap_.insert(std::make_pair(id, 0));
	}

	return runSequenceStepMap_.at(id);
}

// シーケンスステップのセッター
void BehaviorData::SetSequenceStep(int id, int step)
{
	runSequenceStepMap_.at(id) = step;
}

// 初期化
void BehaviorData::Init()
{
	runSequenceStepMap_.clear();
	while (sequenceStack_.size() > 0)
	{
		sequenceStack_.pop();
	}
}