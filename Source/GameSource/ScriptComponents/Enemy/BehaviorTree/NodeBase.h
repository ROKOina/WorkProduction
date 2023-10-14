#pragma once

#include <vector>
#include <string>
#include "BehaviorTree.h"
#include "ActionBase.h"

class JudgmentBase;
class BehaviorData;

// ���������[�N�����p
#define debug_new new(_NORMAL_BLOCK,__FILE__,__LINE__)


// �m�[�h
class NodeBase: public std::enable_shared_from_this<NodeBase>
{
public:
	// �R���X�g���N�^
	NodeBase(int id, std::shared_ptr<NodeBase> parent, std::shared_ptr<NodeBase> sibling, int priority,
		BehaviorTree::SelectRule selectRule, std::shared_ptr<JudgmentBase> judgment, std::shared_ptr<ActionBase> action, int hierarchyNo) :
		id_(id), parent_(parent), sibling_(sibling), priority_(priority),
		selectRule_(selectRule), judgment_(judgment), action_(action), hierarchyNo_(hierarchyNo),
		children_(NULL)
	{
	}

	// �f�X�g���N�^
	~NodeBase();
	//// ���O�Q�b�^�[
	//std::string GetName() { return name; }
	//���ʔԍ�
	int GetId() { return id_; }
	// �e�m�[�h�Q�b�^�[
	std::shared_ptr<NodeBase> GetParent() { return parent_.lock(); }
	// �q�m�[�h�Q�b�^�[
	std::shared_ptr<NodeBase> GetChild(int index);
	// �q�m�[�h�Q�b�^�[(����)
	std::shared_ptr<NodeBase> GetLastChild();
	// �q�m�[�h�Q�b�^�[(�擪)
	std::shared_ptr<NodeBase> GetTopChild();
	// �Z��m�[�h�Q�b�^�[
	std::shared_ptr<NodeBase> GetSibling() { return sibling_.lock(); }
	// �K�w�ԍ��Q�b�^�[
	int GetHirerchyNo() { return hierarchyNo_; }
	// �D�揇�ʃQ�b�^�[
	int GetPriority() { return priority_; }
	// �e�m�[�h�Z�b�^�[
	void SetParent(std::shared_ptr<NodeBase> parent) { this->parent_ = parent; }
	// �q�m�[�h�ǉ�
	void AddChild(std::shared_ptr<NodeBase> child) { children_.push_back(child); }
	// �Z��m�[�h�Z�b�^�[
	void SetSibling(std::shared_ptr<NodeBase> sibling) { this->sibling_ = sibling; }
	// �s���f�[�^�������Ă��邩
	bool HasAction() { return action_ != nullptr ? true : false; }

	//�s���������ŏI������X�e�b�v��
	void EndActionSetStep() { action_->EndActionStep(); }

	// ���s�۔���
	bool Judgment(std::shared_ptr<EnemyCom> enemy);
	// �D�揇�ʑI��
	std::shared_ptr<NodeBase> SelectPriority(std::vector<std::shared_ptr<NodeBase>> list);
	// �����_���I��
	std::shared_ptr<NodeBase> SelectRandom(std::vector<std::shared_ptr<NodeBase>> list);
	// �V�[�P���X�I��
	std::shared_ptr<NodeBase> SelectSequence(std::vector<std::shared_ptr<NodeBase>> list, std::shared_ptr<BehaviorData> data);
	// �m�[�h����
	std::shared_ptr<NodeBase> SearchNode(int searchId);
	// �m�[�h���_
	std::shared_ptr<NodeBase> Inference(std::shared_ptr<EnemyCom> enemy, std::shared_ptr<BehaviorData> data);
	// ���s
	ActionBase::State Run(std::shared_ptr<EnemyCom> enemy, float elapsedTime);
	std::vector<std::shared_ptr<NodeBase>>		children_;		// �q�m�[�h
protected:
	int id_;						//���ʔԍ�
	BehaviorTree::SelectRule	selectRule_;		// �I�����[��
	std::shared_ptr<JudgmentBase> judgment_;		// ����N���X
	std::shared_ptr<ActionBase> action_;			// ���s�N���X
	unsigned int				priority_;		// �D�揇��
	std::weak_ptr<NodeBase> parent_;			// �e�m�[�h
	std::weak_ptr<NodeBase> sibling_;		// �Z��m�[�h
	int							hierarchyNo_;	// �K�w�ԍ�
};