#pragma once

#include "Components\System\Component.h"
#include "GameSource\Character\Character.h"

class PlayerCom : public Component, public Character
{
    //コンポーネントオーバーライド
public:
    PlayerCom() {}
    ~PlayerCom() {}

    // 名前取得
    const char* GetName() const override { return "Player"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;


    //PlayerComクラス
private:
    //移動
    //スティック入力値から移動ベクトルを取得
    DirectX::XMFLOAT3 GetMoveVec();

    //移動入力処理
    bool Move(float elapsedTime);

    //入力値保存
    DirectX::XMFLOAT3 inputMoveVec_;

    //移動パラメーター
    float moveSpeed_ = 5.0f;
    float turnSpeed_ = DirectX::XMConvertToRadians(720);
    float jumpSpeed_ = 20.0f;


    DirectX::XMFLOAT3 up_ = {0,1,0};
    DirectX::XMFLOAT3 look_;

};