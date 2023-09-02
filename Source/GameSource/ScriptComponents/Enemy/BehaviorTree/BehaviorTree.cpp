#include "BehaviorTree.h"
#include "ActionBase.h"
#include "NodeBase.h"
#include "JudgmentBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"


// デストラクタ
BehaviorTree::~BehaviorTree()
{
	NodeAllClear(root_);
}

void BehaviorTree::AddNode(AI_TREE parentId, AI_TREE entryId, int priority, SelectRule selectRule, JudgmentBase* judgment, ActionBase* action)
{
	if (static_cast<int>(parentId) != 0)
	{
		NodeBase* parentNode = root_->SearchNode(static_cast<int>(parentId));

		if (parentNode != nullptr)
		{
			NodeBase* sibling = parentNode->GetLastChild();
			NodeBase* addNode = new NodeBase(static_cast<int>(entryId), parentNode, sibling, priority, selectRule, judgment, action, parentNode->GetHirerchyNo() + 1);

			parentNode->AddChild(addNode);
		}
	}
	else {
		if (root_ == nullptr)
		{
			root_ = new NodeBase(static_cast<int>(entryId), nullptr, nullptr, priority, selectRule, judgment, action, 1);
		}
	}
}

// 推論
NodeBase* BehaviorTree::ActiveNodeInference(BehaviorData* data)
{
	// データをリセットして開始
	data->Init();
	return root_->Inference(owner_, data);
}

// シーケンスノードからの推論開始
NodeBase* BehaviorTree::SequenceBack(NodeBase* sequenceNode, BehaviorData* data)
{
	return sequenceNode->Inference(owner_, data);
}

// ノード実行
NodeBase* BehaviorTree::Run(NodeBase* actionNode, BehaviorData* data, float elapsedTime)
{
	// ノード実行
	ActionBase::State state = actionNode->Run(owner_, elapsedTime);

	// 正常終了
	if (state == ActionBase::State::Complete)
	{
		// シーケンスの途中かを判断
		NodeBase* sequenceNode = data->PopSequenceNode();

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

// 登録されたノードを全て削除する
void BehaviorTree::NodeAllClear(NodeBase* delNode)
{
	size_t count = delNode->children_.size();
	if (count > 0)
	{
		for (NodeBase* node : delNode->children_)
		{
			NodeAllClear(node);
		}
		delete delNode;
	}
	else
	{
		delete delNode;
	}
}

