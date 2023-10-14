#pragma once
#include <string>
#include <memory>

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
	BehaviorTree() :root_(std::shared_ptr<NodeBase>()), owner_(std::shared_ptr<EnemyCom>()) {}
	BehaviorTree(std::shared_ptr<EnemyCom> enemy) :root_(std::shared_ptr<NodeBase>()), owner_(enemy) {}
	~BehaviorTree();

	// ���s�m�[�h�𐄘_����
	std::shared_ptr<NodeBase> ActiveNodeInference(std::shared_ptr<BehaviorData> data);

	// �V�[�P���X�m�[�h���琄�_�J�n
	std::shared_ptr<NodeBase> SequenceBack(std::shared_ptr<NodeBase> sequenceNode, std::shared_ptr<BehaviorData> data);

	// �m�[�h�ǉ�
	void AddNode(AI_TREE parentId, AI_TREE entryId, int priority, SelectRule selectRule, std::shared_ptr<JudgmentBase> judgment, std::shared_ptr<ActionBase> action);

	// ���s
	std::shared_ptr<NodeBase> Run(std::shared_ptr<NodeBase> actionNode, std::shared_ptr<BehaviorData> data, float elapsedTime);
private:
	// ���[�g�m�[�h
	std::shared_ptr<NodeBase> root_;
	std::weak_ptr<EnemyCom> owner_;
};
