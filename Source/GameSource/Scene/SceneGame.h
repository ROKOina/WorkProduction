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

	float transitionOutTimer_;

	bool gameEndFlag_ = false;
	bool gameStartFlag_ = false;
	float sceneTransitionTimer_;

	std::unique_ptr<PostEffect> postEff_;

	//�J�n�摜
	std::unique_ptr<Sprite> startSprite_;
	float startSpriteSize_;

	//BGM
	std::unique_ptr<AudioSource> BGM_ = Audio::Instance().LoadAudioSource("Data/Audio/gameBGM.wav");
};
