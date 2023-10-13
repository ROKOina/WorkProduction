#pragma once

#include <vector>
#include <memory>

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

	// �G�o�^
	void Register(std::shared_ptr<GameObject> enemy);


private:
	struct EnemyData	//�G���
	{
		int index;	//���ʔԍ�
		std::weak_ptr<GameObject> enemy;	//�G
	};
	//�G���܂Ƃ߂�
	std::vector<EnemyData> enemies;

	int currentIndex = 0;
};