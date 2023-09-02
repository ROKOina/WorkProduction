#pragma once
#include <string>

class ActionBase;
class JudgmentBase;
class NodeBase;
class BehaviorData;
class EnemyCom;

enum class AI_TREE;

// �r�w�C�r�A�c���[
class BehaviorTree
{
public:
	// �I�����[��
	enum class SelectRule
	{
		Non,				// �������[�m�[�h�p
		Priority,			// �D�揇��
		Sequence,			// �V�[�P���X
		SequentialLooping,	// �V�[�P���V�������[�s���O
		Random,				// �����_��
	};

public:
	BehaviorTree() :root_(nullptr), owner_(nullptr) {}
	BehaviorTree(EnemyCom* enemy) :root_(nullptr), owner_(enemy) {}
	~BehaviorTree();

	// ���s�m�[�h�𐄘_����
	NodeBase* ActiveNodeInference(BehaviorData* data);

	// �V�[�P���X�m�[�h���琄�_�J�n
	NodeBase* SequenceBack(NodeBase* sequenceNode, BehaviorData* data);

	// �m�[�h�ǉ�
	void AddNode(AI_TREE parentId, AI_TREE entryId, int priority, SelectRule selectRule, JudgmentBase* judgment, ActionBase* action);

	// ���s
	NodeBase* Run(NodeBase* actionNode, BehaviorData* data, float elapsedTime);
private:
	// �m�[�h�S�폜
	void NodeAllClear(NodeBase* delNode);
private:
	// ���[�g�m�[�h
	NodeBase* root_;
	EnemyCom* owner_;
};
