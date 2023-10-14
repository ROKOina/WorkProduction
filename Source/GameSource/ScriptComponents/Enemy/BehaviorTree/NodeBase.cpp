#include "JudgmentBase.h"
#include "NodeBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"
#include "ActionBase.h"



// �f�X�g���N�^
NodeBase::~NodeBase()
{
}
// �q�m�[�h�Q�b�^�[
std::shared_ptr<NodeBase> NodeBase::GetChild(int index)
{
	if (children_.size() <= index)
	{
		return nullptr;
	}
	return children_.at(index);
}

// �q�m�[�h�Q�b�^�[(����)
std::shared_ptr<NodeBase> NodeBase::GetLastChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(children_.size() - 1);
}

// �q�m�[�h�Q�b�^�[(�擪)
std::shared_ptr<NodeBase> NodeBase::GetTopChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(0);
}


// �m�[�h����
std::shared_ptr<NodeBase> NodeBase::SearchNode(int searchId)
{
	if (id_ == searchId)
	{
		return shared_from_this();
	}
	else {
		// �q�m�[�h�Ō���
		for (auto& child : children_)
		{
			std::shared_ptr<NodeBase> node = child->SearchNode(searchId);
			if (node)
				return node;
		}
	}

	return nullptr;
}

// �m�[�h���_
std::shared_ptr<NodeBase> NodeBase::Inference(std::shared_ptr<EnemyCom> enemy, std::shared_ptr<BehaviorData> data)
{
	std::vector<std::shared_ptr<NodeBase>> list;
	std::shared_ptr<NodeBase> result = nullptr;

	// children�̐��������[�v���s���B
	for (auto& child : children_)
	{
		if (child->judgment_ != nullptr)
		{
			// list��child��ǉ����Ă���
			if(child->judgment_->Judgment())list.emplace_back(child);
		}
		else
		{
			//����N���X���Ȃ���Ζ������ɒǉ�
			list.emplace_back(child);
		}
	}

	// �I�����[���Ńm�[�h����
	switch (selectRule_)
	{
		// �D�揇��
	case BehaviorTree::SelectRule::Priority:
		result = SelectPriority(list);
		break;
		// �����_��
	case BehaviorTree::SelectRule::Random:
		result = SelectRandom(list);
		break;
		// �V�[�P���X
	case BehaviorTree::SelectRule::Sequence:
	case BehaviorTree::SelectRule::SequentialLooping:
		result = SelectSequence(list, data);
		break;
	}

	if (result != nullptr)
	{
		// �s��������ΏI��
		if (result->HasAction() == true)
		{
			return result;
		}
		else {
			// ���܂����m�[�h�Ő��_�J�n
			result = result->Inference(enemy, data);
		}
	}

	if (result)
		if (result->HasAction())
			result->action_->StartActionStep();

	return result;
}

// �D�揇�ʂŃm�[�h�I��
std::shared_ptr<NodeBase> NodeBase::SelectPriority(std::vector< std::shared_ptr<NodeBase>> list)
{
	std::shared_ptr<NodeBase> selectNode = nullptr;
	int priority = INT_MAX;

	//��ԗD�揇�ʂ������m�[�h��T����selectNode�Ɋi�[
	for (auto& l : list)
	{
		if (priority > l->GetPriority()) {
			selectNode = l;
			priority = l->GetPriority();
		}
	}

	return selectNode;
}


// �����_���Ńm�[�h�I��
std::shared_ptr<NodeBase> NodeBase::SelectRandom(std::vector<std::shared_ptr<NodeBase>> list)
{
	int selectNo = 0;
	//list�̃T�C�Y�ŗ������擾����selectNo�Ɋi�[
	selectNo = rand() % list.size();

	// list��selectNo�Ԗڂ̎��Ԃ����^�[��
	return list[selectNo];
}

// �V�[�P���X�E�V�[�P���V�������[�s���O�Ńm�[�h�I��
std::shared_ptr<NodeBase> NodeBase::SelectSequence(std::vector<std::shared_ptr<NodeBase>> list, std::shared_ptr<BehaviorData> data)
{
	int step = 0;

	// �w�肳��Ă��钆�ԃm�[�h�̂��V�[�P���X���ǂ��܂Ŏ��s���ꂽ���擾����
	step = data->GetSequenceStep(id_);

	// ���ԃm�[�h�ɓo�^����Ă���m�[�h���ȏ�̏ꍇ�A
	if (step >= children_.size())
	{
		//�ŏ�������s���邽�߁Astep��0����
		if (selectRule_ == BehaviorTree::SelectRule::SequentialLooping)step = 0;
		//���Ɏ��s�ł���m�[�h���Ȃ����߁Anullptr�����^�[��
		if (selectRule_ == BehaviorTree::SelectRule::Sequence)return nullptr;
	}
	// ���s�\���X�g�ɓo�^����Ă���f�[�^�̐��������[�v���s��
	for (auto& l : list)
	{
		// �q�m�[�h�����s�\���X�g�Ɋ܂܂�Ă��邩
		if (children_.at(step)->GetId() == l->GetId())
		{
			data->PushSequenceNode(shared_from_this());
			data->SetSequenceStep(id_, step + 1);
			return l;
		}
	}
	// �w�肳�ꂽ���ԃm�[�h�Ɏ��s�\�m�[�h���Ȃ��̂�nullptr�����^�[������
	return nullptr;
}

// ����
bool NodeBase::Judgment(std::shared_ptr<EnemyCom> enemy)
{
	if (judgment_ != nullptr)
	{
		return Judgment(enemy);
	}
	return true;
}

// �m�[�h���s
ActionBase::State NodeBase::Run(std::shared_ptr<EnemyCom> enemy, float elapsedTime)
{
	if (action_ != nullptr)
	{
		return action_->Run(elapsedTime);
	}

	return ActionBase::State::Failed;
}
