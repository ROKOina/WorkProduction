#include "BehaviorTree.h"
#include "ActionBase.h"
#include "NodeBase.h"
#include "JudgmentBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"


// �f�X�g���N�^
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

// ���_
NodeBase* BehaviorTree::ActiveNodeInference(BehaviorData* data)
{
	// �f�[�^�����Z�b�g���ĊJ�n
	data->Init();
	return root_->Inference(owner_, data);
}

// �V�[�P���X�m�[�h����̐��_�J�n
NodeBase* BehaviorTree::SequenceBack(NodeBase* sequenceNode, BehaviorData* data)
{
	return sequenceNode->Inference(owner_, data);
}

// �m�[�h���s
NodeBase* BehaviorTree::Run(NodeBase* actionNode, BehaviorData* data, float elapsedTime)
{
	// �m�[�h���s
	ActionBase::State state = actionNode->Run(owner_, elapsedTime);

	// ����I��
	if (state == ActionBase::State::Complete)
	{
		// �V�[�P���X�̓r�����𔻒f
		NodeBase* sequenceNode = data->PopSequenceNode();

		// �r������Ȃ��Ȃ�I��
		if (sequenceNode == nullptr)
		{
			return nullptr;
		}
		else {
			// �r���Ȃ炻������n�߂�
			return SequenceBack(sequenceNode, data);
		}
		// ���s�͏I��
	}
	else if (state == ActionBase::State::Failed) {
		return nullptr;
	}

	// ����ێ�
	return actionNode;
}

// �o�^���ꂽ�m�[�h��S�č폜����
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

