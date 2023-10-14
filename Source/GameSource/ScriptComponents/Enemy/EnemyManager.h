#pragma once

#include <vector>
#include <memory>
#include "TelegramEnemy.h"

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

	//敵情報
	enum class EnemyKind
	{
		NEAR_ENEMY,	//近接
		FAR_ENEMY,	//遠隔
	};

	// 敵登録
	void Register(std::shared_ptr<GameObject> enemy, EnemyKind enemyKind);

	//プレイヤー登録
	void RegisterPlayer(std::shared_ptr<GameObject> player)
	{
		player_ = player;
	}

	//IDから敵をゲット
	std::shared_ptr<GameObject> GetEnemyFromId(int id);

	//AI関係

private:
	// メッセージ受信したときの処理
	bool OnMessage(const Telegram& telegram);
public:
	// メッセージ送信関数
	void SendMessaging(int sender, int receiver, MESSAGE_TYPE msg);

	enum AI_ID
	{
		AI_INDEX,
		ENEMY_INDEX,
	};

private:

struct NearEnemyLevel	//近接敵の集まり方のデザイン
{
	float radius = 3;	//プレイヤーの半径
	int inRadiusCount = 3;	//半径に何人入れるか

	int togetherAttackCount = 2;	//同時に攻撃できる回数
};

struct FarEnemyLevel	//遠隔敵の集まり方のデザイン
{

};

public:
	const NearEnemyLevel& GetNearEnemyLevel()const { return nearEnemyLevel; }
	const FarEnemyLevel& GetFarEnemyLevel()const { return farEnemyLevel; }

private:
	struct EnemyData	//敵情報
	{
		std::weak_ptr<GameObject> enemy;	//敵
	};

	//敵をまとめる
	std::vector<EnemyData> nearEnemies_;	//近接
	const NearEnemyLevel nearEnemyLevel;

	std::vector<EnemyData> farEnemies_;	//遠隔
	FarEnemyLevel farEnemyLevel;

	//プレイヤー情報を多用に使うため登録
	std::weak_ptr<GameObject> player_;

	int currentIndex_ = 0;
};