#pragma once

#include "Components\System\Component.h"

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

private:
    float angleX_ = 0, angleY_ = 0;
    float range_ = 4;   //カメラ距離
    DirectX::XMFLOAT3 oldCameraPos_;
    DirectX::XMFLOAT3 Eye_;
};