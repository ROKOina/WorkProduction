#pragma once

#include "Graphics/Sprite/Sprite.h"
#include "Graphics\Shaders\PostEffect.h"
#include "Scene.h"

#include "Audio\AudioSource.h"
#include "Audio\Audio.h"

//タイトルシーン
class SceneTitle :public Scene
{
public:
    SceneTitle(){}
    ~SceneTitle()override
    { }

    //初期化
    void Initialize()override;

    //終了化
    void Finalize()override;

    //更新処理
    void Update(float elapsedTime)override;

    //描画処理
    void Render(float elapsedTime)override;

private:
    //演出
    void TitleProductionUpdate(float elapsedTime);

private:
    std::unique_ptr<PostEffect> postEff_;

    bool firstFrameSkip_ = false;
    bool isSceneEndFlag_ = false;
    bool startFlag_ = false;
    bool isStageMove_ = true;   //ステージ動かすか
    bool isStopFlag_ = false;   //アップ後止める
    float stopTime_ = -1.0f;    //アップ後止める時間

    float eyeTime_ = 3;
    bool isShapeEye_ = false;
    float shapeEye_ = 0;    //瞬き用

    int candySizeCount_ = 1;

    struct StartAfterData
    {
        float playerEulerY = -90;
        float lookCameraPosY = 65.4f;
        DirectX::XMFLOAT3 cameraPos1 = { -0.768f,-0.12f,-0.14f };
        DirectX::XMFLOAT3 cameraPos2 = { -2.4f,0.4f,-10.0f };

    }startAfter_;

    struct CandyData    //ロリポップ
    {
        std::string nodeName;
        DirectX::XMFLOAT3 pos = { 0,0,0 };
        DirectX::XMFLOAT3 angle = { 0,0,0 };
        DirectX::XMFLOAT3 scale = { 1,1,1 };
    };
    CandyData candyData_[2];
    int candyID_ = 0;

    //遷移パーティクルが画面いっぱいになるまでのタイマー
    float transitionOutTimer_ = 1;

    float transitionInTimer_;

    //描画最初初期の姿移るので、遅らせる
    bool frameDelayRender_ = false;

    //BGM
    std::unique_ptr<AudioSource> BGM_ = Audio::Instance().LoadAudioSource("Data/Audio/titleBGM.wav");
    //SE
    std::unique_ptr<AudioSource> pushSE_ = Audio::Instance().LoadAudioSource("Data/Audio/push.wav");
};