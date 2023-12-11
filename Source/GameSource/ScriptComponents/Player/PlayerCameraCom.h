#pragma once

#include "Components\System\Component.h"

#define DefaultRange 4.0f
#define NearRange 2.0f
#define FarRange 7.0f

class PlayerCameraCom :public Component
{
    //コンポーネントオーバーライド
public:
    PlayerCameraCom() {}
    ~PlayerCameraCom() {}

    // 名前取得
    const char* GetName() const override { return "PlayerCamera"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

public:
    void SetAngleY(float angle) { angleY_ = angle; }
    float GetAngleY() { return angleY_; }
    void SetAngleX(float angle) { angleX_ = angle; }
    float GetAngleX() { return angleX_; }

    void SetRange(float range) { range_ = range; }
    float GetRange() { return range_; }
    void SetChangeRange(float time, float range) {
        rangeTimer_ = time;
        rangeDir_ = range;
    }

    void SetForcusPos(DirectX::XMFLOAT3 focus) { lerpFocusPos_ = focus; }
    DirectX::XMFLOAT3 GetForcusPos() { return lerpFocusPos_; }

    void SetIsJust(bool flag) { isJust_ = flag; }
    void SetJustPos(DirectX::XMFLOAT3 pos) { justPos_ = pos; }

private:
    float angleX_ = 0, angleY_ = 0;
    float range_ = 4;   //カメラ距離
    float rangeTimer_;  //カメラの距離を変える時間
    float rangeDir_;    //カメラの距離を指定
    float angleLimit_ = 22;   //カメラ角度制限
    DirectX::XMFLOAT3 oldCameraPos_;

    DirectX::XMFLOAT3 lerpFocusPos_;
    float lerpSpeed_ = 1;
    DirectX::XMFLOAT3 Eye_;

    //ジャスト回避中演出よう
    bool isJust_ = false;
    DirectX::XMFLOAT3 justPos_;
};