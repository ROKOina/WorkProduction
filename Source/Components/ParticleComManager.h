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
		DOOM_SWEETS,
		SITA_WAKU,

		MAX_EFFECT,
	};
	void SetEffect(EFFECT_ID id, DirectX::XMFLOAT3 pos = { 0,0,0 }, std::shared_ptr<GameObject> parent = nullptr);

	void OnGui();

private:
	//パーティクル情報を保存
	struct InializeParticle
	{
		InializeParticle(std::string name, int max = 1000, int sweets = false)
		{
			particleName = name;
			maxParticle = max;
			setSweets = sweets;
		}
		std::string particleName;
		int maxParticle = 1000;
		bool setSweets = false;
	};

	const InializeParticle iniParticle_[MAX_EFFECT] =
	{
		InializeParticle("Data/Effect/para/doomSweets.ipff",1000,true),
		InializeParticle("Data/Effect/para/sitaWaku.ipff"),
	};

    std::vector<std::weak_ptr<GameObject>> particles_;
};