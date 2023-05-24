#pragma once

#include "Player.h"
#include "CameraController.h"
#include "Scene.h"
#include "Graphics\Sprite.h"
#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

#include "Graphics\compute\particle.h"

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
	void Render()override;


private:
	Player* player = nullptr;
	CameraController* cameraController = nullptr;
	std::unique_ptr<AudioSource> BGM=Audio::Instance().LoadAudioSource("Data/Audio/BGM.wav");;


	std::unique_ptr<Particle> particle;
};
