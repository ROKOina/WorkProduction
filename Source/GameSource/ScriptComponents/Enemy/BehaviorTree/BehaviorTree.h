#pragma once
#include <string>

class ActionBase;
class JudgmentBase;
class NodeBase;
class BehaviorData;
class EnemyCom;

enum class AI_TREE;

// ビヘイビアツリー
class BehaviorTree
{
public:
	// 選択ルール
	enum class SelectRule
	{
		Non,				// 無い末端ノード用
		Priority,			// 優先順位
		Sequence,			// シーケンス
		SequentialLooping,	// シーケンシャルルーピング
		Random,				// ランダム
	};

public:
	BehaviorTree() :root_(nullptr), owner_(nullptr) {}
	BehaviorTree(EnemyCom* enemy) :root_(nullptr), owner_(enemy) {}
	~BehaviorTree();

	// 実行ノードを推論する
	NodeBase* ActiveNodeInference(BehaviorData* data);

	// シーケンスノードから推論開始
	NodeBase* SequenceBack(NodeBase* sequenceNode, BehaviorData* data);

	// ノード追加
	void AddNode(AI_TREE parentId, AI_TREE entryId, int priority, SelectRule selectRule, JudgmentBase* judgment, ActionBase* action);

	// 実行
	NodeBase* Run(NodeBase* actionNode, BehaviorData* data, float elapsedTime);
private:
	// ノード全削除
	void NodeAllClear(NodeBase* delNode);
private:
	// ルートノード
	NodeBase* root_;
	EnemyCom* owner_;
};
