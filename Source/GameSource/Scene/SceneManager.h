#pragma once

#include "Scene.h"

class GameObject;

//シーンマネージャー
class SceneManager
{
private:
    SceneManager();
    ~SceneManager(){}

public:
    //唯一のインスタンス取得
    static SceneManager& Instance()
    {
        static SceneManager instance;
        return instance;
    }

    //更新処理
    void Update(float elapsedTime);

    //描画処理
    void Render(float elapsedTime);

    //シーンクリア
    void Clear();

    //シーン切り替え
    void ChangeScene(Scene* scene);

    void ChangeParticleUpdate(float elapsedTime);
    void ChangeParticleRender();
    void ChangeParticleGui();

    std::shared_ptr<GameObject> GetParticleObj()
    {
        return changeParticle_;
    }

private:
    Scene* currentScene_ = nullptr;
    Scene* nextScene_ = nullptr;

    std::shared_ptr<GameObject> changeParticle_;
    std::shared_ptr<GameObject> changeParticleCamera_;
};