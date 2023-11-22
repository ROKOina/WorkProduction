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
	// �B��̃C���X�^���X�擾
	static ParticleComManager& Instance()
	{
		static ParticleComManager instance;
		return instance;
	}

	void Update(float elapsedTime);

	//�ǂ̃p�[�e�B�N�����o����
	enum EFFECT_ID
	{
		SWORD_SWEETS,
		COMBO_1,
		COMBO_1_FIRE,
		COMBO_2,
		COMBO_3,
		COMBO_TAME,
		DAMAGE_SWETTS,
		DAMAGE_SWETTS_ENEMY,
		JUST_SPARK,
		JUST_UNDER,

		MAX_EFFECT,
	};
	//pos:�o���ʒu�@parent:�q�ɂ���@posObj:�I�u�W�F�N�g�̈ʒu�ɒǏ]����(bone�{�[���w��,offset������Γ����)
	std::shared_ptr<GameObject> SetEffect(EFFECT_ID id, DirectX::XMFLOAT3 pos = { 0,0,0 }
	, std::shared_ptr<GameObject> parent = nullptr, std::shared_ptr<GameObject> posObj = nullptr, std::string boneName ="", DirectX::XMFLOAT3 offsetPos = {0,0,0});

	void OnGui();

private:
	//�p�[�e�B�N������ۑ�
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
		InializeParticle("Data/Effect/para/combo1fire.ipff"),
		InializeParticle("Data/Effect/para/combo2.ipff"),
		InializeParticle("Data/Effect/para/combo3.ipff", true, 1500, true),
		InializeParticle("Data/Effect/para/combotame.ipff", false),
		InializeParticle("Data/Effect/para/damageSweets.ipff", true, 100, true),
		InializeParticle("Data/Effect/para/damageSweetsEnemy.ipff", false, 10, true),
		InializeParticle("Data/Effect/para/justSpark.ipff",true,100),
		InializeParticle("Data/Effect/para/justSpawnUnder.ipff", false, 500, true),
	};

	struct ParticleDataMove
	{
		std::weak_ptr<GameObject> particle;
		std::weak_ptr<GameObject> posObj;	//�Ǐ]����I�u�W�F�N�g
		std::string boneName;				//�{�[���ɒǏ]����ꍇ
		DirectX::XMFLOAT3 offsetPos;		//�Ǐ]�̃I�t�Z�b�g	
	};

    std::vector<ParticleDataMove> particles_;
};