#include "BehaviorData.h"
#include "NodeBase.h"


// �V�[�P���X�m�[�h�̃|�b�v
std::shared_ptr<NodeBase> BehaviorData::PopSequenceNode()
{
	// ��Ȃ�NULL
	if (sequenceStack_.empty() != 0)
	{
		return nullptr;
	}
	std::shared_ptr<NodeBase> node = sequenceStack_.top().lock();
	if (node != nullptr)
	{
		// ���o�����f�[�^���폜
		sequenceStack_.pop();
	}
	return node;
}

// �V�[�P���X�X�e�b�v�̃Q�b�^�[
int BehaviorData::GetSequenceStep(int id)
{
	if (runSequenceStepMap_.count(id) == 0)
	{
		runSequenceStepMap_.insert(std::make_pair(id, 0));
	}

	return runSequenceStepMap_.at(id);
}

// �V�[�P���X�X�e�b�v�̃Z�b�^�[
void BehaviorData::SetSequenceStep(int id, int step)
{
	runSequenceStepMap_.at(id) = step;
}

// ������
void BehaviorData::Init()
{
	runSequenceStepMap_.clear();
	while (sequenceStack_.size() > 0)
	{
		sequenceStack_.pop();
	}
}