#pragma once

#include <vector>
#include <stack>
#include <map>
#include "BehaviorTree.h"

class NodeBase;

// Behavior�ۑ��f�[�^
class BehaviorData
{
public:
	// �R���X�g���N�^
	BehaviorData() { Init(); }
	// �V�[�P���X�m�[�h�̃v�b�V��
	void PushSequenceNode(std::shared_ptr<NodeBase> node) { sequenceStack_.push(node); }
	// �V�[�P���X�m�[�h�̃|�b�v
	std::shared_ptr<NodeBase> PopSequenceNode();
	// �V�[�P���X�X�e�b�v�̃Q�b�^�[
	int GetSequenceStep(int id);
	// �V�[�P���X�X�e�b�v�̃Z�b�^�[
	void SetSequenceStep(int id, int step);
	// ������
	void Init();
private:
	std::stack<std::weak_ptr<NodeBase>> sequenceStack_;				// ���s���钆�ԃm�[�h���X�^�b�N
	std::map<int, int> runSequenceStepMap_;		// ���s���̒��ԃm�[�h�̃X�e�b�v���L�^
};
