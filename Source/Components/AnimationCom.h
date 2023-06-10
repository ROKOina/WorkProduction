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
public:

    //アニメーション再生
    void PlayAnimation(int index, bool loop, float blendSeconds = 0.2f);

    //アニメーション再生中か
    bool IsPlayAnimation();

    //現在のアニメーション再生時間取得
    float GetCurrentAnimationSecoonds()const { return currentAnimationSeconds_; }

    //アニメーション追加
    void ImportFbxAnimation(const char* filename);

private:
    int currentAnimationIndex_ = -1;
    float currentAnimationSeconds_ = 0.0f;
    bool animationLoopFlag_ = false;
    bool animationEndFlag_ = false;
    float animationBlendTime_ = 0.0f;
    float animationBlendSeconds_ = 0.0f;
};