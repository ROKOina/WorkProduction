#pragma once

#include <vector>
#include <stack>
#include <map>
#include "BehaviorTree.h"

class NodeBase;

// Behavior保存データ
class BehaviorData
{
public:
	// コンストラクタ
	BehaviorData() { Init(); }
	// シーケンスノードのプッシュ
	void PushSequenceNode(std::shared_ptr<NodeBase> node) { sequenceStack_.push(node); }
	// シーケンスノードのポップ
	std::shared_ptr<NodeBase> PopSequenceNode();
	// シーケンスステップのゲッター
	int GetSequenceStep(int id);
	// シーケンスステップのセッター
	void SetSequenceStep(int id, int step);
	// 初期化
	void Init();
private:
	std::stack<std::weak_ptr<NodeBase>> sequenceStack_;				// 実行する中間ノードをスタック
	std::map<int, int> runSequenceStepMap_;		// 実行中の中間ノードのステップを記録
};
