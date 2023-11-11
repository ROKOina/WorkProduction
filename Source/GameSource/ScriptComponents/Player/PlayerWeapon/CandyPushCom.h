#pragma once

#include "Components\System\Component.h"

class PushWeaponCom : public Component
{
    //コンポーネントオーバーライド
public:
    PushWeaponCom() {}
    ~PushWeaponCom() {}

    // 名前取得
    const char* GetName() const override { return "PushWeapon"; }

    // 開始処理
    void Start() override {};

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;

public:
    void MoveStart(DirectX::XMFLOAT3 movePos,DirectX::XMFLOAT3 startPos);
    bool IsMove() { return isMove_; }

private:
    bool isMove_ = false;
    DirectX::XMFLOAT3 startPos_;
    DirectX::XMFLOAT3 movePos_;
    float speed_ = 60;

    DirectX::XMFLOAT3 saveEuler_;
};