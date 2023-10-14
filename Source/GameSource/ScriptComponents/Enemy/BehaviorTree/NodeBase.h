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
class NodeBase: public std::enable_shared_from_this<NodeBase>
{
public:
	// コンストラクタ
	NodeBase(int id, std::shared_ptr<NodeBase> parent, std::shared_ptr<NodeBase> sibling, int priority,
		BehaviorTree::SelectRule selectRule, std::shared_ptr<JudgmentBase> judgment, std::shared_ptr<ActionBase> action, int hierarchyNo) :
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
	std::shared_ptr<NodeBase> GetParent() { return parent_.lock(); }
	// 子ノードゲッター
	std::shared_ptr<NodeBase> GetChild(int index);
	// 子ノードゲッター(末尾)
	std::shared_ptr<NodeBase> GetLastChild();
	// 子ノードゲッター(先頭)
	std::shared_ptr<NodeBase> GetTopChild();
	// 兄弟ノードゲッター
	std::shared_ptr<NodeBase> GetSibling() { return sibling_.lock(); }
	// 階層番号ゲッター
	int GetHirerchyNo() { return hierarchyNo_; }
	// 優先順位ゲッター
	int GetPriority() { return priority_; }
	// 親ノードセッター
	void SetParent(std::shared_ptr<NodeBase> parent) { this->parent_ = parent; }
	// 子ノード追加
	void AddChild(std::shared_ptr<NodeBase> child) { children_.push_back(child); }
	// 兄弟ノードセッター
	void SetSibling(std::shared_ptr<NodeBase> sibling) { this->sibling_ = sibling; }
	// 行動データを持っているか
	bool HasAction() { return action_ != nullptr ? true : false; }

	//行動を強制で終了するステップに
	void EndActionSetStep() { action_->EndActionStep(); }

	// 実行可否判定
	bool Judgment(std::shared_ptr<EnemyCom> enemy);
	// 優先順位選択
	std::shared_ptr<NodeBase> SelectPriority(std::vector<std::shared_ptr<NodeBase>> list);
	// ランダム選択
	std::shared_ptr<NodeBase> SelectRandom(std::vector<std::shared_ptr<NodeBase>> list);
	// シーケンス選択
	std::shared_ptr<NodeBase> SelectSequence(std::vector<std::shared_ptr<NodeBase>> list, std::shared_ptr<BehaviorData> data);
	// ノード検索
	std::shared_ptr<NodeBase> SearchNode(int searchId);
	// ノード推論
	std::shared_ptr<NodeBase> Inference(std::shared_ptr<EnemyCom> enemy, std::shared_ptr<BehaviorData> data);
	// 実行
	ActionBase::State Run(std::shared_ptr<EnemyCom> enemy, float elapsedTime);
	std::vector<std::shared_ptr<NodeBase>>		children_;		// 子ノード
protected:
	int id_;						//識別番号
	BehaviorTree::SelectRule	selectRule_;		// 選択ルール
	std::shared_ptr<JudgmentBase> judgment_;		// 判定クラス
	std::shared_ptr<ActionBase> action_;			// 実行クラス
	unsigned int				priority_;		// 優先順位
	std::weak_ptr<NodeBase> parent_;			// 親ノード
	std::weak_ptr<NodeBase> sibling_;		// 兄弟ノード
	int							hierarchyNo_;	// 階層番号
};