#include "SceneManager.h"

//更新処理
void SceneManager::Update(float elapsedTime)
{
    if (nextScene_ != nullptr)
    {
        //古いシーンを終了処理
        Clear();

        //新しいシーンを設定
        currentScene_ = nextScene_;
        nextScene_ = nullptr;

        //シーン初期化処理
        if (!currentScene_->IsReady())currentScene_->Initialize();

    }

    if (currentScene_ != nullptr)
    {
        currentScene_->Update(elapsedTime);
    }
}

//描画処理
void SceneManager::Render()
{
    if (currentScene_ != nullptr)
    {
        currentScene_->Render();
    }
}

//シーンクリア
void SceneManager::Clear()
{
    if (currentScene_ != nullptr)
    {
        currentScene_->Finalize();
        delete currentScene_;
        currentScene_ = nullptr;
    }
}

//シーン切り替え
void SceneManager::ChangeScene(Scene* scene)
{
    //新しいシーンを設定
    nextScene_ = scene;
}
