#pragma once

#include <vector>
#include <memory>
#include "TelegramEnemy.h"

class GameObject;

//�G�l�~�[�}�l�[�W���[
class EnemyManager
{
private:
	EnemyManager() {}
	~EnemyManager() {}

public:

	// �B��̃C���X�^���X�擾
	static EnemyManager& Instance()
	{
		static EnemyManager instance;
		return instance;
	}

	// �X�V����
	void Update(float elapsedTime);

	//�G���
	enum class EnemyKind
	{
		NEAR_ENEMY,	//�ߐ�
		FAR_ENEMY,	//���u
	};

	// �G�o�^
	void Register(std::shared_ptr<GameObject> enemy, EnemyKind enemyKind);

	//�v���C���[�o�^
	void RegisterPlayer(std::shared_ptr<GameObject> player)
	{
		player_ = player;
	}

	//ID����G���Q�b�g
	std::shared_ptr<GameObject> GetEnemyFromId(int id);

	//AI�֌W

private:
	// ���b�Z�[�W��M�����Ƃ��̏���
	bool OnMessage(const Telegram& telegram);
public:
	// ���b�Z�[�W���M�֐�
	void SendMessaging(int sender, int receiver, MESSAGE_TYPE msg);

	enum AI_ID
	{
		AI_INDEX,
		ENEMY_INDEX,
	};

private:

struct NearEnemyLevel	//�ߐړG�̏W�܂���̃f�U�C��
{
	float radius = 3;	//�v���C���[�̔��a
	int inRadiusCount = 3;	//���a�ɉ��l����邩

	int togetherAttackCount = 2;	//�����ɍU���ł����
};

struct FarEnemyLevel	//���u�G�̏W�܂���̃f�U�C��
{

};

public:
	const NearEnemyLevel& GetNearEnemyLevel()const { return nearEnemyLevel; }
	const FarEnemyLevel& GetFarEnemyLevel()const { return farEnemyLevel; }

private:
	struct EnemyData	//�G���
	{
		std::weak_ptr<GameObject> enemy;	//�G
	};

	//�G���܂Ƃ߂�
	std::vector<EnemyData> nearEnemies_;	//�ߐ�
	const NearEnemyLevel nearEnemyLevel;

	std::vector<EnemyData> farEnemies_;	//���u
	FarEnemyLevel farEnemyLevel;

	//�v���C���[���𑽗p�Ɏg�����ߓo�^
	std::weak_ptr<GameObject> player_;

	int currentIndex_ = 0;
};