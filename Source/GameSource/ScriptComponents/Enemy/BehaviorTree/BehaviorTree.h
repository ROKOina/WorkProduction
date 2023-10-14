#pragma once
#include <string>
#include <memory>

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
	BehaviorTree() :root_(std::shared_ptr<NodeBase>()), owner_(std::shared_ptr<EnemyCom>()) {}
	BehaviorTree(std::shared_ptr<EnemyCom> enemy) :root_(std::shared_ptr<NodeBase>()), owner_(enemy) {}
	~BehaviorTree();

	// 実行ノードを推論する
	std::shared_ptr<NodeBase> ActiveNodeInference(std::shared_ptr<BehaviorData> data);

	// シーケンスノードから推論開始
	std::shared_ptr<NodeBase> SequenceBack(std::shared_ptr<NodeBase> sequenceNode, std::shared_ptr<BehaviorData> data);

	// ノード追加
	void AddNode(AI_TREE parentId, AI_TREE entryId, int priority, SelectRule selectRule, std::shared_ptr<JudgmentBase> judgment, std::shared_ptr<ActionBase> action);

	// 実行
	std::shared_ptr<NodeBase> Run(std::shared_ptr<NodeBase> actionNode, std::shared_ptr<BehaviorData> data, float elapsedTime);
private:
	// ルートノード
	std::shared_ptr<NodeBase> root_;
	std::weak_ptr<EnemyCom> owner_;
};
