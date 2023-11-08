#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Scene.h"
#include <thread>

//ローディングシーン
class SceneLoading :public Scene
{
public:
    SceneLoading(Scene* nextScene):nextScene_(nextScene){}
    ~SceneLoading(){}

    //初期化
    void Initialize()override;

    //終了化
    void Finalize()override;

    //更新処理
    void Update(float elapsedTime)override;

    //描画処理
    void Render(float elapsedTime)override;

private:
    //ローディングスレッド
    static void LoadingThread(SceneLoading* scene);

private :
    Sprite* sprite_ = nullptr;
    float angle_ = 0.0f;
    Scene* nextScene_ = nullptr;
    std::thread* thread_ = nullptr;
};