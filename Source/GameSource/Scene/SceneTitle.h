#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Graphics\Shaders\PostEffect.h"
#include "Scene.h"

//タイトルシーン
class SceneTitle :public Scene
{
public:
    SceneTitle(){}
    ~SceneTitle()override{}

    //初期化
    void Initialize()override;

    //終了化
    void Finalize()override;

    //更新処理
    void Update(float elapsedTime)override;

    //描画処理
    void Render()override;

private:
    std::unique_ptr<PostEffect> postEff_;
    Sprite* sprite_ = nullptr;

    bool firstAnimation_ = false;
    bool isSceneEndFlag_ = false;
    bool startFlag_ = false;
    bool isStageMove_ = true;   //ステージ動かすか
    bool isStopFlag_ = false;   //アップ後止める
    float stopTime_ = -1.0f;    //アップ後止める時間

    float eyeTime_ = 3;
    bool isShapeEye_ = false;
    float shapeEye_ = 0;    //瞬き用

    struct StartAfterData
    {
        float playerEulerY = -82;
        float lookCameraPosY = 65.4f;
        DirectX::XMFLOAT3 cameraPos1 = { -0.768f,-0.12f,-0.14f };
        DirectX::XMFLOAT3 cameraPos2 = { -1.8f,0.4f,-10.0f };

    }startAfter_;
};