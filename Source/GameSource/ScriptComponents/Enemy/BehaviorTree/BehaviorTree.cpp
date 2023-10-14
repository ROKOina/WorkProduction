#include "BehaviorTree.h"
#include "ActionBase.h"
#include "NodeBase.h"
#include "JudgmentBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"


// �f�X�g���N�^
BehaviorTree::~BehaviorTree()
{
}

void BehaviorTree::AddNode(AI_TREE parentId, AI_TREE entryId, int priority, SelectRule selectRule, std::shared_ptr<JudgmentBase> judgment, std::shared_ptr<ActionBase> action)
{
	if (static_cast<int>(parentId) != 0)
	{
		std::shared_ptr<NodeBase> parentNode = root_->SearchNode(static_cast<int>(parentId));

		if (parentNode != nullptr)
		{
			std::shared_ptr<NodeBase> sibling = parentNode->GetLastChild();
			std::shared_ptr<NodeBase> addNode = std::make_shared<NodeBase>(static_cast<int>(entryId), parentNode, sibling, priority, selectRule, judgment, action, parentNode->GetHirerchyNo() + 1);

			parentNode->AddChild(addNode);
		}
	}
	else {
		if (root_ == nullptr)
		{
			root_ = std::make_shared<NodeBase>(static_cast<int>(entryId), nullptr, nullptr, priority, selectRule, judgment, action, 1);
		}
	}
}

// ���_
std::shared_ptr<NodeBase> BehaviorTree::ActiveNodeInference(std::shared_ptr<BehaviorData> data)
{
	// �f�[�^�����Z�b�g���ĊJ�n
	data->Init();
	return root_->Inference(owner_.lock(), data);
}

// �V�[�P���X�m�[�h����̐��_�J�n
std::shared_ptr<NodeBase> BehaviorTree::SequenceBack(std::shared_ptr<NodeBase> sequenceNode, std::shared_ptr<BehaviorData> data)
{
	return sequenceNode->Inference(owner_.lock(), data);
}

// �m�[�h���s
std::shared_ptr<NodeBase> BehaviorTree::Run(std::shared_ptr<NodeBase> actionNode, std::shared_ptr<BehaviorData> data, float elapsedTime)
{
	// �m�[�h���s
	ActionBase::State state = actionNode->Run(owner_.lock(), elapsedTime);

	// ����I��
	if (state == ActionBase::State::Complete)
	{
		// �V�[�P���X�̓r�����𔻒f
		std::shared_ptr<NodeBase> sequenceNode = data->PopSequenceNode();

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

