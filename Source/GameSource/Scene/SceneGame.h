#pragma once

#include "Scene.h"
#include "Graphics\Sprite\Sprite.h"
#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

#include "Graphics\Shaders\PostEffect.h"

// ゲームシーン
class SceneGame :public Scene
{
public:
	SceneGame() {}
	~SceneGame()override {}

	// 初期化
	void Initialize()override;

	// 終了化
	void Finalize()override;

	// 更新処理
	void Update(float elapsedTime)override;

	// 描画処理
	void Render(float elapsedTime)override;


private:

	float transitionOutTimer_;

	bool gameEndFlag_ = false;
	bool gameStartFlag_ = false;
	float sceneTransitionTimer_;

	std::unique_ptr<PostEffect> postEff_;

	//開始画像
	std::unique_ptr<Sprite> startSprite_;
	float startSpriteSize_;

	//BGM
	std::unique_ptr<AudioSource> BGM_ = Audio::Instance().LoadAudioSource("Data/Audio/gameBGM.wav");
};
