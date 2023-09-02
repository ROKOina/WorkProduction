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
class NodeBase
{
public:
	// �R���X�g���N�^
	NodeBase(int id, NodeBase* parent, NodeBase* sibling, int priority,
		BehaviorTree::SelectRule selectRule, JudgmentBase* judgment, ActionBase* action, int hierarchyNo) :
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
	NodeBase* GetParent() { return parent_; }
	// �q�m�[�h�Q�b�^�[
	NodeBase* GetChild(int index);
	// �q�m�[�h�Q�b�^�[(����)
	NodeBase* GetLastChild();
	// �q�m�[�h�Q�b�^�[(�擪)
	NodeBase* GetTopChild();
	// �Z��m�[�h�Q�b�^�[
	NodeBase* GetSibling() { return sibling_; }
	// �K�w�ԍ��Q�b�^�[
	int GetHirerchyNo() { return hierarchyNo_; }
	// �D�揇�ʃQ�b�^�[
	int GetPriority() { return priority_; }
	// �e�m�[�h�Z�b�^�[
	void SetParent(NodeBase* parent) { this->parent_ = parent; }
	// �q�m�[�h�ǉ�
	void AddChild(NodeBase* child) { children_.push_back(child); }
	// �Z��m�[�h�Z�b�^�[
	void SetSibling(NodeBase* sibling) { this->sibling_ = sibling; }
	// �s���f�[�^�������Ă��邩
	bool HasAction() { return action_ != nullptr ? true : false; }
	// ���s�۔���
	bool Judgment(EnemyCom* enemy);
	// �D�揇�ʑI��
	NodeBase* SelectPriority(std::vector<NodeBase*>* list);
	// �����_���I��
	NodeBase* SelectRandom(std::vector<NodeBase*>* list);
	// �V�[�P���X�I��
	NodeBase* SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data);
	// �m�[�h����
	NodeBase* SearchNode(int searchId);
	// �m�[�h���_
	NodeBase* Inference(EnemyCom* enemy, BehaviorData* data);
	// ���s
	ActionBase::State Run(EnemyCom* enemy, float elapsedTime);
	std::vector<NodeBase*>		children_;		// �q�m�[�h
protected:
	int id_;						//���ʔԍ�
	BehaviorTree::SelectRule	selectRule_;		// �I�����[��
	JudgmentBase* judgment_;		// ����N���X
	ActionBase* action_;			// ���s�N���X
	unsigned int				priority_;		// �D�揇��
	NodeBase* parent_;			// �e�m�[�h
	NodeBase* sibling_;		// �Z��m�[�h
	int							hierarchyNo_;	// �K�w�ԍ�
};