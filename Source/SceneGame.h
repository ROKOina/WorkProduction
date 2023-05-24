#pragma once

#include "Player.h"
#include "CameraController.h"
#include "Scene.h"
#include "Graphics\Sprite.h"
#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

#include "Graphics\compute\particle.h"

// �Q�[���V�[��
class SceneGame :public Scene
{
public:
	SceneGame() {}
	~SceneGame()override {}

	// ������
	void Initialize()override;

	// �I����
	void Finalize()override;

	// �X�V����
	void Update(float elapsedTime)override;

	// �`�揈��
	void Render()override;


private:
	Player* player = nullptr;
	CameraController* cameraController = nullptr;
	std::unique_ptr<AudioSource> BGM=Audio::Instance().LoadAudioSource("Data/Audio/BGM.wav");;


	std::unique_ptr<Particle> particle;
};
