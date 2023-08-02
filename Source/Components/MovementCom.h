#pragma once

#include "System\Component.h"

class MovementCom : public Component
{
    //�R���|�[�l���g�I�[�o�[���C�h
public:
    MovementCom() {}
    ~MovementCom() {}

    // ���O�擾
    const char* GetName() const override { return "Movement"; }

    // �J�n����
    void Start() override;

    // �X�V����
    void Update(float elapsedTime) override;

    // GUI�`��
    void OnGUI() override;


    //Movement�N���X
private:
    //�c�����ړ��X�V
    void VerticalUpdate(float elapsedTime);
    //�������ړ��X�V
    void HorizonUpdate(float elapsedTime);
    //���͂��X�V
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

    //�d��
    const float& GetGravity()const {
        return gravity_;
    }
    void SetGravity(float gravity) {
        gravity_ = gravity;
    }

    //�ڒn����
    const bool OnGround() { return onGround_; }

    //���C
    const float& GetFriction()const {
        return friction_;
    }
    void SetFriction(float friction) {
        friction_ = friction;
    }

    //�ő呬�x
    const float& GetMoveMaxSpeed()const {
        return moveMaxSpeed_;
    }
    void SetMoveMaxSpeed(float moveMaxSpeed) {
        moveMaxSpeed_ = moveMaxSpeed;
    }

    //�����x
    const float& GetMoveAcceleration()const {
        return moveAcceleration_;
    }
    void SetMoveAcceleration(float moveAcceleration) {
        moveAcceleration_ = moveAcceleration;
    }

private:

    //����
    DirectX::XMFLOAT3 velocity_ = { 0,0,0 };
    DirectX::XMFLOAT3 nonMaxSpeedVelocity_ = { 0,0,0 }; //�ő�X�s�[�h�𖳎���������

    float gravity_ = -0.25f;  //�d��
    bool onGround_ = false;      //�n�ʂɂ��Ă��邩
    float friction_ = 0.8f;  //���C

    float moveMaxSpeed_ = 10.0f;
    float moveAcceleration_ = 1.0f;
};