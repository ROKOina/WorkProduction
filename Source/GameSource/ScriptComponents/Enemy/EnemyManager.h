#pragma once

#include <vector>
#include <memory>
#include "TelegramEnemy.h"

#include "Graphics/Sprite/Sprite.h"

class GameObject;

//エネミーマネージャー
class EnemyManager
{
private:
	EnemyManager() 
	{
		enemyCount_ = std::make_unique<Sprite>("./Data/Sprite/suuzi.png");
	}
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

	//sprite描画
	void Render2D(float elapsedTime);

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

	//敵のスピードを一括で変更(secondsは何秒かけてもどるか)
	void SetEnemySpeed(float speed, float seconds);

	//スロー処理しているか
	bool GetIsSlow() { return isSlow_; }

	//エネミーのアップデート止める
	bool GetIsUpdateFlag() { return isUpdate_; }
	void SetIsUpdateFlag(bool flag) { isUpdate_ = flag; }

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

	//スロー
	bool isSlow_ = false;
	float slowSeconds_;
	float slowTimer_;
	float slowSpeed_;

	//エネミーのアップデートするか
	bool isUpdate_ = true;

	//UI
	std::unique_ptr<Sprite> enemyCount_;
	int killCount_;
	DirectX::XMFLOAT4 killCountSprite_ = { 762.2f,-1.2f,0.5f,70.0f }; //xy:pos z:size w:間隔
};