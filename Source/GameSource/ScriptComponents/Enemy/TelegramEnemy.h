#pragma once

// ���M���郁�b�Z�[�W�̎��
enum class MESSAGE_TYPE : int
{
	//AI->Enemy
	MsgGiveAttackRight,			// �U������^����
	MsgGiveNearRight,			// �ڋߌ���^����

	//Enemy->AI
	MsgChangeAttackRight,		// �U�����̈ڏ�
	MsgAskAttackRight,			// �U������v��
	MsgAskNearRight,			// �ڋ߂�v��
};

class Telegram
{
public:
	int sender;			// ������ID
	int receiver;		// �󂯂Ă�ID
	MESSAGE_TYPE msg;	// ���b�Z�[�WNO

	//�R���X�g���N�^
	Telegram(int sender, int receiver, MESSAGE_TYPE msg) :sender(sender), receiver(receiver), msg(msg)
	{}
};
