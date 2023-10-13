#pragma once

#include <vector>
#include <memory>

class GameObject;

//エネミーマネージャー
class EnemyManager
{
private:
	EnemyManager() {}
	~EnemyManager() {}

public:

	// 唯一のインスタンス取得
	static EnemyManager& Instance()
	{
		static EnemyManager instance;
		return instance;
	}

	// 更新処理
	void Update(float elapsedTime);

	// 敵登録
	void Register(std::shared_ptr<GameObject> enemy);


private:
	struct EnemyData	//敵情報
	{
		int index;	//識別番号
		std::weak_ptr<GameObject> enemy;	//敵
	};
	//敵をまとめる
	std::vector<EnemyData> enemies;

	int currentIndex = 0;
};