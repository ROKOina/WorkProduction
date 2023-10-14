#include "BehaviorTree.h"
#include "ActionBase.h"
#include "NodeBase.h"
#include "JudgmentBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"


// デストラクタ
BehaviorTree::~BehaviorTree()
{
}

void BehaviorTree::AddNode(AI_TREE parentId, AI_TREE entryId, int priority, SelectRule selectRule, std::shared_ptr<JudgmentBase> judgment, std::shared_ptr<ActionBase> action)
{
	if (static_cast<int>(parentId) != 0)
	{
		std::shared_ptr<NodeBase> parentNode = root_->SearchNode(static_cast<int>(parentId));

		if (parentNode != nullptr)
		{
			std::shared_ptr<NodeBase> sibling = parentNode->GetLastChild();
			std::shared_ptr<NodeBase> addNode = std::make_shared<NodeBase>(static_cast<int>(entryId), parentNode, sibling, priority, selectRule, judgment, action, parentNode->GetHirerchyNo() + 1);

			parentNode->AddChild(addNode);
		}
	}
	else {
		if (root_ == nullptr)
		{
			root_ = std::make_shared<NodeBase>(static_cast<int>(entryId), nullptr, nullptr, priority, selectRule, judgment, action, 1);
		}
	}
}

// 推論
std::shared_ptr<NodeBase> BehaviorTree::ActiveNodeInference(std::shared_ptr<BehaviorData> data)
{
	// データをリセットして開始
	data->Init();
	return root_->Inference(owner_.lock(), data);
}

// シーケンスノードからの推論開始
std::shared_ptr<NodeBase> BehaviorTree::SequenceBack(std::shared_ptr<NodeBase> sequenceNode, std::shared_ptr<BehaviorData> data)
{
	return sequenceNode->Inference(owner_.lock(), data);
}

// ノード実行
std::shared_ptr<NodeBase> BehaviorTree::Run(std::shared_ptr<NodeBase> actionNode, std::shared_ptr<BehaviorData> data, float elapsedTime)
{
	// ノード実行
	ActionBase::State state = actionNode->Run(owner_.lock(), elapsedTime);

	// 正常終了
	if (state == ActionBase::State::Complete)
	{
		// シーケンスの途中かを判断
		std::shared_ptr<NodeBase> sequenceNode = data->PopSequenceNode();

		// 途中じゃないなら終了
		if (sequenceNode == nullptr)
		{
			return nullptr;
		}
		else {
			// 途中ならそこから始める
			return SequenceBack(sequenceNode, data);
		}
		// 失敗は終了
	}
	else if (state == ActionBase::State::Failed) {
		return nullptr;
	}

	// 現状維持
	return actionNode;
}

