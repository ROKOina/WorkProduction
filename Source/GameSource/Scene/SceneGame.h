#pragma once

#include "Scene.h"
#include "Graphics\Sprite\Sprite.h"
#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

#include "Graphics\Shaders\PostEffect.h"

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
	void Render(float elapsedTime)override;


private:
	std::unique_ptr<AudioSource> bgm_=Audio::Instance().LoadAudioSource("Data/Audio/BGM.wav");

	bool gameEndFlag_ = false;
	bool gameStartFlag_ = false;

	std::unique_ptr<PostEffect> postEff_;
};
