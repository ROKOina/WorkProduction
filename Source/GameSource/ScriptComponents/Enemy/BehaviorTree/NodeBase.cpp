#include "JudgmentBase.h"
#include "NodeBase.h"
#include "../EnemyCom.h"
#include "BehaviorData.h"
#include "ActionBase.h"



// �f�X�g���N�^
NodeBase::~NodeBase()
{
	delete judgment_;
	delete action_;
}
// �q�m�[�h�Q�b�^�[
NodeBase* NodeBase::GetChild(int index)
{
	if (children_.size() <= index)
	{
		return nullptr;
	}
	return children_.at(index);
}

// �q�m�[�h�Q�b�^�[(����)
NodeBase* NodeBase::GetLastChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(children_.size() - 1);
}

// �q�m�[�h�Q�b�^�[(�擪)
NodeBase* NodeBase::GetTopChild()
{
	if (children_.size() == 0)
	{
		return nullptr;
	}

	return children_.at(0);
}


// �m�[�h����
NodeBase* NodeBase::SearchNode(int searchId)
{
	if (id_ == searchId)
	{
		return this;
	}
	else {
		// �q�m�[�h�Ō���
		for (auto itr = children_.begin(); itr != children_.end(); itr++)
		{
			NodeBase* ret = (*itr)->SearchNode(searchId);

			if (ret != nullptr)
			{
				return ret;
			}
		}
	}

	return nullptr;
}

// �m�[�h���_
NodeBase* NodeBase::Inference(EnemyCom* enemy, BehaviorData* data)
{
	std::vector<NodeBase*> list;
	NodeBase* result = nullptr;

	// children�̐��������[�v���s���B
	for (int i = 0; i < children_.size(); i++)
	{
		if (children_.at(i)->judgment_ != nullptr)
		{
			// list��children.at(i)��ǉ����Ă���
			if (children_.at(i)->judgment_->Judgment())list.emplace_back(children_.at(i));
		}
		else {
			//����N���X���Ȃ���Ζ������ɒǉ�
			list.emplace_back(children_.at(i));
		}
	}

	// �I�����[���Ńm�[�h����
	switch (selectRule_)
	{
		// �D�揇��
	case BehaviorTree::SelectRule::Priority:
		result = SelectPriority(&list);
		break;
		// �����_��
	case BehaviorTree::SelectRule::Random:
		result = SelectRandom(&list);
		break;
		// �V�[�P���X
	case BehaviorTree::SelectRule::Sequence:
	case BehaviorTree::SelectRule::SequentialLooping:
		result = SelectSequence(&list, data);
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

	return result;
}

// �D�揇�ʂŃm�[�h�I��
NodeBase* NodeBase::SelectPriority(std::vector<NodeBase*>* list)
{
	NodeBase* selectNode = nullptr;
	int priority = INT_MAX;

	//��ԗD�揇�ʂ������m�[�h��T����selectNode�Ɋi�[
	for (auto& l : *list)
	{
		if (priority > l->GetPriority()) {
			selectNode = l;
			priority = l->GetPriority();
		}
	}

	return selectNode;
}


// �����_���Ńm�[�h�I��
NodeBase* NodeBase::SelectRandom(std::vector<NodeBase*>* list)
{
	int selectNo = 0;
	//list�̃T�C�Y�ŗ������擾����selectNo�Ɋi�[
	selectNo = rand() % list->size();

	// list��selectNo�Ԗڂ̎��Ԃ����^�[��
	return (*list).at(selectNo);
}

// �V�[�P���X�E�V�[�P���V�������[�s���O�Ńm�[�h�I��
NodeBase* NodeBase::SelectSequence(std::vector<NodeBase*>* list, BehaviorData* data)
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
	for (auto itr = list->begin(); itr != list->end(); itr++)
	{
		// �q�m�[�h�����s�\���X�g�Ɋ܂܂�Ă��邩
		if (children_.at(step)->GetId() == (*itr)->GetId())
		{
			data->PushSequenceNode(this);
			data->SetSequenceStep(id_, step + 1);
			return *itr;
		}
	}
	// �w�肳�ꂽ���ԃm�[�h�Ɏ��s�\�m�[�h���Ȃ��̂�nullptr�����^�[������
	return nullptr;
}

// ����
bool NodeBase::Judgment(EnemyCom* enemy)
{
	if (judgment_ != nullptr)
	{
		return Judgment(enemy);
	}
	return true;
}

// �m�[�h���s
ActionBase::State NodeBase::Run(EnemyCom* enemy, float elapsedTime)
{
	if (action_ != nullptr)
	{
		return action_->Run(elapsedTime);
	}

	return ActionBase::State::Failed;
}
