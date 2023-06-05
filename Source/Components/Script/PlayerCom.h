#pragma once

#include "../System\Component.h"
#include "GameSource\Character\Character.h"

class PlayerCom : public Component,public Character
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
    //カメラ
    void CameraControll(float elapsedTime);

    //カメラパラメーター
    DirectX::XMFLOAT3   target = { 0,0,0 };
    DirectX::XMFLOAT3   angle = { 0,0,0 };
    float               rollSpeed = DirectX::XMConvertToRadians(90);
    float               range = 10.0f;
    float               maxAngleX = DirectX::XMConvertToRadians(45);
    float               minAngleX = DirectX::XMConvertToRadians(-45);



    //移動
    //スティック入力値から移動ベクトルを取得
    DirectX::XMFLOAT3 GetMoveVec();

    //移動入力処理
    bool InputMove(float elapsedTime);

    //移動パラメーター
    float moveSpeed = 5.0f;
    float turnSpeed = DirectX::XMConvertToRadians(720);
    float jumpSpeed = 20.0f;

};