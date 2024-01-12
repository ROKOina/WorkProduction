#pragma once

typedef unsigned long long BIG_LONG;

class Score
{
private:
    Score(){}
    ~Score(){}

public:
	// �B��̃C���X�^���X�擾
	static Score& Instance()
	{
		static Score instance;
		return instance;
	}

	//�X�R�A�����Z�b�g����
	void ResetScore() 
	{
		score_ = 0;
		maxCombo_ = 0;
	}

	void AddScore(int num)
	{
		score_ += num;
	}

	BIG_LONG score_ = 0;
	int maxCombo_ = 0;
};