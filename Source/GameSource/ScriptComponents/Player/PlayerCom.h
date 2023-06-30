#pragma once

#include "Components\System\Component.h"
#include "GameSource\Character\Character.h"

class PlayerCom : public Component/*, public Character*/
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

    void AddForce(const DirectX::XMFLOAT3& force) {
        velocity_.x += force.x;
        velocity_.y += force.y;
        velocity_.z += force.z;
    };

    //PlayerComクラス
private:
    //移動
    //スティック入力値から移動ベクトルを取得
    DirectX::XMFLOAT3 GetMoveVec();

    //移動入力処理
    bool IsMove(float elapsedTime);
    //回転
    void Trun(float elapsedTime);
    //縦方向移動
    void VerticalMove();
    //横方向移動
    void HorizonMove();
    //縦方向移動更新
    void VerticalUpdate(float elapsedTime);
    //横方向移動更新
    void HorizonUpdate(float elapsedTime);

    //ダッシュ
    void DashMove(float elapsedTime);

    //速力を更新
    void VelocityAppPosition(float elapsedTime);


    //入力値保存
    DirectX::XMFLOAT3 inputMoveVec_;

    //移動パラメーター
    DirectX::XMFLOAT3 velocity_ = { 0,0,0 };

    //走りと歩きを切り替える
    enum MOVE_PARAM
    {
        WALK,
        RUN,
        MAX,
    };
    struct
    {
        float moveMaxSpeed = 10.0f;
        float moveAcceleration = 0.2f;
        float turnSpeed = 8.0f;
    }moveParam_[MOVE_PARAM::MAX];
    int moveParamType = MOVE_PARAM::WALK;

    //ダッシュ用の速力
    DirectX::XMFLOAT3 dashVelocity_ = { 0,0,0 };
    //ダッシュ時にプラスで速くなる
    float dashSpeed_ = 10.0f;
    bool isDash_ = false;

    float jumpSpeed_ = 20.0f;
    float gravity_   = -0.25f;  //重力
    float friction_ = 0.1f;  //摩擦

    DirectX::XMFLOAT3 up_ = {0,1,0};
};