#pragma once

#include <DirectXMath.h>

#include "System\Component.h"

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
    float GetCurrentAnimationIndex()const { return currentAnimationIndex_; }

    //アニメーション追加
    void ImportFbxAnimation(const char* filename);

    //アニメーションイベント取得(ポジションと再生中か)
    bool GetCurrentAnimationEvent(const char* eventName, DirectX::XMFLOAT3& position);

    //アニメーションイベント取得
    const AnimEvent GetAnimationEvent(const char* eventName) const{
        for (AnimEvent anim : currentAnimationEvents_)
        {
            if (std::strcmp(eventName, anim.name.c_str()) == 0)
            {
                return anim;
            }
        }
    }

private:
    int currentAnimationIndex_ = -1;
    float currentAnimationSeconds_ = 0.0f;
    bool animationLoopFlag_ = false;
    bool animationEndFlag_ = false;
    float animationBlendTime_ = 0.0f;
    float animationBlendSeconds_ = 0.0f;

    bool isLooped_ = false;  //ループしたか

    std::vector<AnimEvent> currentAnimationEvents_;
};