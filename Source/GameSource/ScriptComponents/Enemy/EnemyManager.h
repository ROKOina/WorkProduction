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

	//GUI
	void OnGui();

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

	
	//近接敵の攻撃カウント取得
	int GetCurrentNearAttackCount();

	//近接敵の接近カウント取得
	int GetCurrentNearFlagCount();

	//近接敵の経路探査カウント取得
	int GetCurrentNearPathCount();


	//敵の数取得
	int GetEnemyCount();

	//IDから敵をゲット
	std::shared_ptr<GameObject> GetEnemyFromId(int id);

	//空の敵を削除
	void EraseExpiredEnemy();

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

public:

struct NearEnemyLevel	//近接敵の集まり方のデザイン
{
	float radius = 5;	//プレイヤー中心の半径
	int inRadiusCount = 3;	//半径に何人入れるか

	int togetherAttackCount = 2;	//同時に攻撃できる回数

	int togetherPathCount = 2;	//同時に経路探査できる回数
};

struct FarEnemyLevel	//遠隔敵の集まり方のデザイン
{

};

	const NearEnemyLevel& GetNearEnemyLevel()const { return nearEnemyLevel_; }
	const FarEnemyLevel& GetFarEnemyLevel()const { return farEnemyLevel_; }


	struct EnemyData	//敵情報
	{
		std::weak_ptr<GameObject> enemy;	//敵
	};
	std::vector<EnemyData>& GetNearEnemies() { return nearEnemies_; }
	std::vector<EnemyData>& GetFarEnemies() { return farEnemies_; }

private:

	//敵をまとめる
	std::vector<EnemyData> nearEnemies_;	//近接
	const NearEnemyLevel nearEnemyLevel_;

	std::vector<EnemyData> farEnemies_;	//遠隔
	FarEnemyLevel farEnemyLevel_;

	//プレイヤー情報を多用に使うため登録
	std::weak_ptr<GameObject> player_;

	int currentIndex_ = 0;

};