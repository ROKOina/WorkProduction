#include "JudgmentBase.h"
#include "NodeBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"
#include "ActionBase.h"



// デストラクタ
NodeBase::~NodeBase()
{
	delete judgment_;
	delete action_;
}
// 子ノードゲッター
NodeBase* NodeBase::GetChild(int index)
{
	if (children_.size() <= index)
	{
		return nullptr;
	}
	return children_.at(index);
}

// 子ノードゲッター(末尾)
NodeBase* NodeBase::GetLastChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(children_.size() - 1);
}

// 子ノードゲッター(先頭)
NodeBase* NodeBase::GetTopChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(0);
}


// ノード検索
NodeBase* NodeBase::SearchNode(int searchId)
{
	if (id_ == searchId)
	{
		return this;
	}
	else {
		// 子ノードで検索
		for (auto itr = children_.begin(); itr != children_.end(); itr++)
		{
			NodeBase* ret = (*itr)->SearchNode(searchId);

			if (ret != nullptr)
			{
				return ret;
			}
		}
	}

	return nullptr;
}

// ノード推論
NodeBase* NodeBase::Inference(EnemyCom* enemy, BehaviorData* data)
{
	std::vector<NodeBase*> list;
	NodeBase* result = nullptr;

	// childrenの数だけループを行う。
	for (int i = 0; i < children_.size(); i++)
	{
		if (children_.at(i)->judgment_ != nullptr)
		{
			// listにchildren.at(i)を追加していく
			if (children_.at(i)->judgment_->Judgment())list.emplace_back(children_.at(i));
		}
		else {
			//判定クラスがなければ無条件に追加
			list.emplace_back(children_.at(i));
		}
	}

	// 選択ルールでノード決め
	switch (selectRule_)
	{
		// 優先順位
	case BehaviorTree::SelectRule::Priority:
		result = SelectPriority(&list);
		break;
		// ランダム
	case BehaviorTree::SelectRule::Random:
		result = SelectRandom(&list);
		break;
		// シーケンス
	case BehaviorTree::SelectRule::Sequence:
	case BehaviorTree::SelectRule::SequentialLooping:
		result = SelectSequence(&list, data);
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

	return result;
}

// 優先順位でノード選択
NodeBase* NodeBase::SelectPriority(std::vector<NodeBase*>* list)
{
	NodeBase* selectNode = nullptr;
	int priority = INT_MAX;

	//一番優先順位が高いノードを探してselectNodeに格納
	for (auto& l : *list)
	{
		if (priority > l->GetPriority()) {
			selectNode = l;
			priority = l->GetPriority();
		}
	}

	return selectNode;
}


// ランダムでノード選択
NodeBase* NodeBase::SelectRandom(std::vector<NodeBase*>* list)
{
	int selectNo = 0;
	//listのサイズで乱数を取得してselectNoに格納
	selectNo = rand() % list->size();

	// listのselectNo番目の実態をリターン
	return (*list).at(selectNo);
}

// シーケンス・シーケンシャルルーピングでノード選択
NodeBase* NodeBase::SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data)
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
	for (auto itr = list->begin(); itr != list->end(); itr++)
	{
		// 子ノードが実行可能リストに含まれているか
		if (children_.at(step)->GetId() == (*itr)->GetId())
		{
			data->PushSequenceNode(this);
			data->SetSequenceStep(id_, step + 1);
			return *itr;
		}
	}
	// 指定された中間ノードに実行可能ノードがないのでnullptrをリターンする
	return nullptr;
}

// 判定
bool NodeBase::Judgment(EnemyCom* enemy)
{
	if (judgment_ != nullptr)
	{
		return Judgment(enemy);
	}
	return true;
}

// ノード実行
ActionBase::State NodeBase::Run(EnemyCom* enemy, float elapsedTime)
{
	if (action_ != nullptr)
	{
		return action_->Run(elapsedTime);
	}

	return ActionBase::State::Failed;
}
