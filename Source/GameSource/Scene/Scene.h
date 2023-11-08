#pragma once

#include <memory>

//前方宣言
class CameraCom;

//シーン
class Scene
{
public:
    Scene(){}
    virtual ~Scene(){}

    //初期化
    virtual void Initialize() = 0;

    //終了化
    virtual void Finalize() = 0;

    //更新処理
    virtual void Update(float elapsedTime) = 0;

    //描画処理
    virtual void Render(float elapsedTime) = 0;

    //準備完了しているか
    bool IsReady()const { return isReady_; }

    //準備完了設定
    void SetReady() { isReady_ = true; }

protected:
    //メインカメラ
    std::shared_ptr<CameraCom> mainCamera_;

private:
    bool isReady_ = false;
};