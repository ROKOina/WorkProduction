#include "JudgmentBase.h"
#include "NodeBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"
#include "ActionBase.h"



// デストラクタ
NodeBase::~NodeBase()
{
}
// 子ノードゲッター
std::shared_ptr<NodeBase> NodeBase::GetChild(int index)
{
	if (children_.size() <= index)
	{
		return nullptr;
	}
	return children_.at(index);
}

// 子ノードゲッター(末尾)
std::shared_ptr<NodeBase> NodeBase::GetLastChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(children_.size() - 1);
}

// 子ノードゲッター(先頭)
std::shared_ptr<NodeBase> NodeBase::GetTopChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(0);
}


// ノード検索
std::shared_ptr<NodeBase> NodeBase::SearchNode(int searchId)
{
	if (id_ == searchId)
	{
		return shared_from_this();
	}
	else {
		// 子ノードで検索
		for (auto& child : children_)
		{
			std::shared_ptr<NodeBase> node = child->SearchNode(searchId);
			if (node)
				return node;
		}
	}

	return nullptr;
}

// ノード推論
std::shared_ptr<NodeBase> NodeBase::Inference(std::shared_ptr<EnemyCom> enemy, std::shared_ptr<BehaviorData> data)
{
	std::vector<std::shared_ptr<NodeBase>> list;
	std::shared_ptr<NodeBase> result = nullptr;

	// childrenの数だけループを行う。
	for (auto& child : children_)
	{
		if (child->judgment_ != nullptr)
		{
			// listにchildを追加していく
			if(child->judgment_->Judgment())list.emplace_back(child);
		}
		else
		{
			//判定クラスがなければ無条件に追加
			list.emplace_back(child);
		}
	}

	// 選択ルールでノード決め
	switch (selectRule_)
	{
		// 優先順位
	case BehaviorTree::SelectRule::Priority:
		result = SelectPriority(list);
		break;
		// ランダム
	case BehaviorTree::SelectRule::Random:
		result = SelectRandom(list);
		break;
		// シーケンス
	case BehaviorTree::SelectRule::Sequence:
	case BehaviorTree::SelectRule::SequentialLooping:
		result = SelectSequence(list, data);
		break;
	}

	if (result != nullptr)
	{
		// 行動があれば終了
		if (result->HasAction() == true)
		{
			return result;
		}
		else {
			// 決まったノードで推論開始
			result = result->Inference(enemy, data);
		}
	}

	if (result)
		if (result->HasAction())
			result->action_->StartActionStep();

	return result;
}

// 優先順位でノード選択
std::shared_ptr<NodeBase> NodeBase::SelectPriority(std::vector< std::shared_ptr<NodeBase>> list)
{
	std::shared_ptr<NodeBase> selectNode = nullptr;
	int priority = INT_MAX;

	//一番優先順位が高いノードを探してselectNodeに格納
	for (auto& l : list)
	{
		if (priority > l->GetPriority()) {
			selectNode = l;
			priority = l->GetPriority();
		}
	}

	return selectNode;
}


// ランダムでノード選択
std::shared_ptr<NodeBase> NodeBase::SelectRandom(std::vector<std::shared_ptr<NodeBase>> list)
{
	int selectNo = 0;
	//listのサイズで乱数を取得してselectNoに格納
	selectNo = rand() % list.size();

	// listのselectNo番目の実態をリターン
	return list[selectNo];
}

// シーケンス・シーケンシャルルーピングでノード選択
std::shared_ptr<NodeBase> NodeBase::SelectSequence(std::vector<std::shared_ptr<NodeBase>> list, std::shared_ptr<BehaviorData> data)
{
	int step = 0;

	// 指定されている中間ノードのがシーケンスがどこまで実行されたか取得する
	step = data->GetSequenceStep(id_);

	// 中間ノードに登録されているノード数以上の場合、
	if (step >= children_.size())
	{
		//最初から実行するため、stepに0を代入
		if (selectRule_ == BehaviorTree::SelectRule::SequentialLooping)step = 0;
		//次に実行できるノードがないため、nullptrをリターン
		if (selectRule_ == BehaviorTree::SelectRule::Sequence)return nullptr;
	}
	// 実行可能リストに登録されているデータの数だけループを行う
	for (auto& l : list)
	{
		// 子ノードが実行可能リストに含まれているか
		if (children_.at(step)->GetId() == l->GetId())
		{
			data->PushSequenceNode(shared_from_this());
			data->SetSequenceStep(id_, step + 1);
			return l;
		}
	}
	// 指定された中間ノードに実行可能ノードがないのでnullptrをリターンする
	return nullptr;
}

// 判定
bool NodeBase::Judgment(std::shared_ptr<EnemyCom> enemy)
{
	if (judgment_ != nullptr)
	{
		return Judgment(enemy);
	}
	return true;
}

// ノード実行
ActionBase::State NodeBase::Run(std::shared_ptr<EnemyCom> enemy, float elapsedTime)
{
	if (action_ != nullptr)
	{
		return action_->Run(elapsedTime);
	}

	return ActionBase::State::Failed;
}
