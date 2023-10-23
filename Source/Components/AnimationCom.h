#pragma once

#include <DirectXMath.h>

#include "System\Component.h"

#include "Graphics\Model\Model.h"
#include "Graphics\Model\ModelResource.h"

//アニメーション
class AnimationCom : public Component
{
    // コンポーネントオーバーライド
public:
    AnimationCom() {}
    ~AnimationCom() {}

    // 名前取得
    const char* GetName() const override { return "Animation"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

    // アニメーション更新
    void AnimationUpdata(float elapsedTime);

    //Animationクラス
private:
    //アニメーションイベント用
    struct AnimEvent
    {
        bool enabled = false;
        std::string name;
        int nodeIndex = -1;
        DirectX::XMFLOAT3 position;
        ModelResource::AnimationEvent resourceEventData;
    };

public:

    //アニメーション再生
    void PlayAnimation(int index, bool loop, float blendSeconds = 0.2f);

    //アニメーション再生中か
    bool IsPlayAnimation();

    //アニメーションループしたか
    bool IsPlayAnimationLooped()const { return isLooped_; }

    //現在のアニメーション再生時間取得
    float GetCurrentAnimationSecoonds()const { return currentAnimationSeconds_; }

    //現在のアニメーションindex取得
    int GetCurrentAnimationIndex()const { return currentAnimationIndex_; }

    //再生速度
    void SetAnimationSpeed(float speed) { animationSpeed_ = speed; }
    const float GetAnimationSpeed()const { return animationSpeed_; }

    //アニメーション追加
    void ImportFbxAnimation(const char* filename);

    //アニメーションイベント取得(ポジションと再生中か)
    bool GetCurrentAnimationEvent(const char* eventName, DirectX::XMFLOAT3& position);
    //アニメーションイベント取得(エンドフレーム後なら)
    bool GetCurrentAnimationEventIsEnd(const char* eventName);
    //アニメーションイベント取得(スタートフレーム前なら)
    bool GetCurrentAnimationEventIsStart(const char* eventName);


    //アニメーションイベント名前取得
    const AnimEvent GetAnimationEvent(const char* eventName) const{
        for (AnimEvent anim : currentAnimationEvents_)
        {
            if (std::strcmp(eventName, anim.name.c_str()) == 0)
            {
                return anim;
            }
        }
        return {};
    }

    //現在のアニメーションイベント取得
    const std::vector<AnimEvent>& GetCurrentAnimationEventsData()const {
        return currentAnimationEvents_;
    }

    //停止するか
    bool GetIsStop() { return isStop_; }
    void SetIsStop(bool stop) { isStop_ = stop; }

private:
    int currentAnimationIndex_ = -1;
    float currentAnimationSeconds_ = 0.0f;
    bool animationLoopFlag_ = false;
    bool animationEndFlag_ = false;
    float animationBlendTime_ = 0.0f;
    float animationBlendSeconds_ = 0.0f;
    float animationSpeed_ = 1.0f;

    //ルート用
    DirectX::XMFLOAT3 oldRootPos_ = {};

    bool isLooped_ = false;  //ループしたか

    bool isStop_ = false;    //停止するか

    std::vector<AnimEvent> currentAnimationEvents_;

    //首のボーンの制御
    Model::Node* neckNode;
};