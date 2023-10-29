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

	//GUI
	void OnGui();

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

	
	//�ߐړG�̍U���J�E���g�擾
	int GetCurrentNearAttackCount();

	//�ߐړG�̐ڋ߃J�E���g�擾
	int GetCurrentNearFlagCount();

	//�ߐړG�̌o�H�T���J�E���g�擾
	int GetCurrentNearPathCount();


	//�G�̐��擾
	int GetEnemyCount();

	//ID����G���Q�b�g
	std::shared_ptr<GameObject> GetEnemyFromId(int id);

	//��̓G���폜
	void EraseExpiredEnemy();

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

public:

struct NearEnemyLevel	//�ߐړG�̏W�܂���̃f�U�C��
{
	float radius = 5;	//�v���C���[���S�̔��a
	int inRadiusCount = 3;	//���a�ɉ��l����邩

	int togetherAttackCount = 2;	//�����ɍU���ł����

	int togetherPathCount = 2;	//�����Ɍo�H�T���ł����
};

struct FarEnemyLevel	//���u�G�̏W�܂���̃f�U�C��
{

};

	const NearEnemyLevel& GetNearEnemyLevel()const { return nearEnemyLevel_; }
	const FarEnemyLevel& GetFarEnemyLevel()const { return farEnemyLevel_; }


	struct EnemyData	//�G���
	{
		std::weak_ptr<GameObject> enemy;	//�G
	};
	std::vector<EnemyData>& GetNearEnemies() { return nearEnemies_; }
	std::vector<EnemyData>& GetFarEnemies() { return farEnemies_; }

private:

	//�G���܂Ƃ߂�
	std::vector<EnemyData> nearEnemies_;	//�ߐ�
	const NearEnemyLevel nearEnemyLevel_;

	std::vector<EnemyData> farEnemies_;	//���u
	FarEnemyLevel farEnemyLevel_;

	//�v���C���[���𑽗p�Ɏg�����ߓo�^
	std::weak_ptr<GameObject> player_;

	int currentIndex_ = 0;

};