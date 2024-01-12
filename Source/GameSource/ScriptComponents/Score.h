#pragma once

typedef unsigned long long BIG_LONG;

class Score
{
private:
    Score(){}
    ~Score(){}

public:
	// 唯一のインスタンス取得
	static Score& Instance()
	{
		static Score instance;
		return instance;
	}

	//スコアをリセットする
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