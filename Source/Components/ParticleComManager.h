#pragma once

#include <memory>
#include <string>
#include <vector>
#include <DirectXMath.h>

class GameObject;

class ParticleComManager
{
private:
    ParticleComManager() {}
    ~ParticleComManager() {}

public:
	// 唯一のインスタンス取得
	static ParticleComManager& Instance()
	{
		static ParticleComManager instance;
		return instance;
	}

	void Update(float elapsedTime);

	//どのパーティクルを出すか
	enum EFFECT_ID
	{
		SWORD_SWEETS,
		COMBO_1,
		COMBO_2,
		COMBO_3,
		DAMAGE_SWETTS,
		DAMAGE_SWETTS_ENEMY,

		MAX_EFFECT,
	};
	//pos:出現位置　parent:子にする　posObj:オブジェクトの位置に追従する(offsetがあれば入れる)
	std::shared_ptr<GameObject> SetEffect(EFFECT_ID id, DirectX::XMFLOAT3 pos = { 0,0,0 }
	, std::shared_ptr<GameObject> parent = nullptr, std::shared_ptr<GameObject> posObj = nullptr, DirectX::XMFLOAT3 offsetPos = { 0,0,0 });

	void OnGui();

private:
	//パーティクル情報を保存
	struct InializeParticle
	{
		InializeParticle(std::string name, bool isAutoDeleteRoop = true, int max = 1000, int sweets = false)
		{
			particleName = name;
			isAutoDeleteRoopFlag = isAutoDeleteRoop;
			maxParticle = max;
			setSweets = sweets;
		}
		std::string particleName;
		bool isAutoDeleteRoopFlag = false;
		int maxParticle = 1000;
		bool setSweets = false;
	};

	const InializeParticle iniParticle_[MAX_EFFECT] =
	{
		InializeParticle("Data/Effect/para/SwordOkasi.ipff", false, 100, true),
		InializeParticle("Data/Effect/para/combo1.ipff"),
		InializeParticle("Data/Effect/para/combo2.ipff"),
		InializeParticle("Data/Effect/para/combo3.ipff", true, 1500, true),
		InializeParticle("Data/Effect/para/damageSweets.ipff", true, 100, true),
		InializeParticle("Data/Effect/para/damageSweetsEnemy.ipff", false, 10, true),
	};

	struct ParticleDataMove
	{
		std::weak_ptr<GameObject> particle;
		std::weak_ptr<GameObject> posObj;	//追従するオブジェクト
		DirectX::XMFLOAT3 offsetPos;		//追従のオフセット	
	};

    std::vector<ParticleDataMove> particles_;
};