#pragma once

#include <vector>
#include <string>
#include "BehaviorTree.h"
#include "ActionBase.h"

class JudgmentBase;
class BehaviorData;

// メモリリーク調査用
#define debug_new new(_NORMAL_BLOCK,__FILE__,__LINE__)


// ノード
class NodeBase
{
public:
	// コンストラクタ
	NodeBase(int id, NodeBase* parent, NodeBase* sibling, int priority,
		BehaviorTree::SelectRule selectRule, JudgmentBase* judgment, ActionBase* action, int hierarchyNo) :
		id_(id), parent_(parent), sibling_(sibling), priority_(priority),
		selectRule_(selectRule), judgment_(judgment), action_(action), hierarchyNo_(hierarchyNo),
		children_(NULL)
	{
	}

	// デストラクタ
	~NodeBase();
	//// 名前ゲッター
	//std::string GetName() { return name; }
	//識別番号
	int GetId() { return id_; }
	// 親ノードゲッター
	NodeBase* GetParent() { return parent_; }
	// 子ノードゲッター
	NodeBase* GetChild(int index);
	// 子ノードゲッター(末尾)
	NodeBase* GetLastChild();
	// 子ノードゲッター(先頭)
	NodeBase* GetTopChild();
	// 兄弟ノードゲッター
	NodeBase* GetSibling() { return sibling_; }
	// 階層番号ゲッター
	int GetHirerchyNo() { return hierarchyNo_; }
	// 優先順位ゲッター
	int GetPriority() { return priority_; }
	// 親ノードセッター
	void SetParent(NodeBase* parent) { this->parent_ = parent; }
	// 子ノード追加
	void AddChild(NodeBase* child) { children_.push_back(child); }
	// 兄弟ノードセッター
	void SetSibling(NodeBase* sibling) { this->sibling_ = sibling; }
	// 行動データを持っているか
	bool HasAction() { return action_ != nullptr ? true : false; }
	// 実行可否判定
	bool Judgment(EnemyCom* enemy);
	// 優先順位選択
	NodeBase* SelectPriority(std::vector<NodeBase*>* list);
	// ランダム選択
	NodeBase* SelectRandom(std::vector<NodeBase*>* list);
	// シーケンス選択
	NodeBase* SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data);
	// ノード検索
	NodeBase* SearchNode(int searchId);
	// ノード推論
	NodeBase* Inference(EnemyCom* enemy, BehaviorData* data);
	// 実行
	ActionBase::State Run(EnemyCom* enemy, float elapsedTime);
	std::vector<NodeBase*>		children_;		// 子ノード
protected:
	int id_;						//識別番号
	BehaviorTree::SelectRule	selectRule_;		// 選択ルール
	JudgmentBase* judgment_;		// 判定クラス
	ActionBase* action_;			// 実行クラス
	unsigned int				priority_;		// 優先順位
	NodeBase* parent_;			// 親ノード
	NodeBase* sibling_;		// 兄弟ノード
	int							hierarchyNo_;	// 階層番号
};