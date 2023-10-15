#pragma once

// 送信するメッセージの種類
enum class MESSAGE_TYPE : int
{
	//AI->Enemy
	MsgGiveAttackRight,			// 攻撃権を与える
	MsgGiveNearRight,			// 接近権を与える

	//Enemy->AI
	MsgAskAttackRight,			// 攻撃権を要求
	MsgAskNearRight,			// 接近を要求
};

class Telegram
{
public:
	int sender;			// 送り手のID
	int receiver;		// 受けてのID
	MESSAGE_TYPE msg;	// メッセージNO

	//コンストラクタ
	Telegram(int sender, int receiver, MESSAGE_TYPE msg) :sender(sender), receiver(receiver), msg(msg)
	{}
};
