#pragma once

#include "System\Component.h"

#define GRAVITY_NORMAL -70
#define GRAVITY_FALL -120
#define GRAVITY_ZERO 0

class MovementCom : public Component
{
    //コンポーネントオーバーライド
public:
    MovementCom() {}
    ~MovementCom() {}

    // 名前取得
    const char* GetName() const override { return "Movement"; }

    // 開始処理
    void Start() override;

    // 更新処理
    void Update(float elapsedTime) override;

    // GUI描画
    void OnGUI() override;


    //Movementクラス
private:
    //縦方向移動更新
    void VerticalUpdate(float elapsedTime);
    //横方向移動更新
    void HorizonUpdate(float elapsedTime);
    //速力を更新
    void VelocityAppPosition(float elapsedTime);

public:
    //velocity
    void AddForce(const DirectX::XMFLOAT3& force) {
        velocity_.x += force.x * moveAcceleration_;
        velocity_.y += force.y;
        velocity_.z += force.z * moveAcceleration_;
    }
    void ZeroVelocity() {
        velocity_ = { 0,0,0 };
    }
    void ZeroVelocityY() {
        velocity_ = { velocity_.x,0,velocity_.z };
    }
    const DirectX::XMFLOAT3& GetVelocity()const {
        return velocity_;
   }

    //nonMaxSpeedVelocity
    void AddNonMaxSpeedForce(const DirectX::XMFLOAT3& force) {
        nonMaxSpeedVelocity_.x += force.x;
        velocity_.y += force.y;
        nonMaxSpeedVelocity_.z += force.z;
    }
    void ZeroNonMaxSpeedVelocity() {
        nonMaxSpeedVelocity_ = { 0,0,0 };
    }
    const DirectX::XMFLOAT3& GetNonMaxSpeedVelocity()const {
        return nonMaxSpeedVelocity_;
   }

    //重力
    const float& GetGravity()const {
        return gravity_;
    }
    void SetGravity(float gravity) {
        gravity_ = gravity;
    }

    //接地判定
    const bool OnGround() { return onGround_; }

    //摩擦
    const float& GetFriction()const {
        return friction_;
    }
    void SetFriction(float friction) {
        friction_ = friction;
    }

    //最大速度
    const float& GetMoveMaxSpeed()const {
        return moveMaxSpeed_;
    }
    void SetMoveMaxSpeed(float moveMaxSpeed) {
        moveMaxSpeed_ = moveMaxSpeed;
    }

    //加速度
    const float& GetMoveAcceleration()const {
        return moveAcceleration_;
    }
    void SetMoveAcceleration(float moveAcceleration) {
        moveAcceleration_ = moveAcceleration;
    }

private:

    //速力
    DirectX::XMFLOAT3 velocity_ = { 0,0,0 };
    DirectX::XMFLOAT3 nonMaxSpeedVelocity_ = { 0,0,0 }; //最大スピードを無視した速力

    float gravity_ = GRAVITY_NORMAL;  //重力
    bool onGround_ = false;      //地面についているか
    float friction_ = 15;  //摩擦
    //float friction_ = 0.1f;  //摩擦

    float moveMaxSpeed_ = 10.0f;
    float moveAcceleration_ = 1.0f;
};