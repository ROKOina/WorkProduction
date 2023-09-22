#include "MovementCom.h"

#include <imgui.h>
#include "Components\TransformCom.h"
#include "Graphics\Graphics.h"

// �J�n����
void MovementCom::Start()
{

}

// �X�V����
void MovementCom::Update(float elapsedTime)
{
    VerticalUpdate(elapsedTime);
    HorizonUpdate(elapsedTime);
    VelocityAppPosition(elapsedTime);
};
// GUI�`��
void MovementCom::OnGUI()
{
    ImGui::DragFloat3("velocity", &velocity_.x);
    ImGui::DragFloat3("nonMaxSpeedVelocity_", &nonMaxSpeedVelocity_.x);
    ImGui::DragFloat("gravity", &gravity_, 0.1f);
    ImGui::DragFloat("friction", &friction_, 0.1f);
    ImGui::DragFloat("moveMaxSpeed", &moveMaxSpeed_, 0.1f);
    ImGui::DragFloat("moveAcceleration", &moveAcceleration_, 0.1f);
}

//�c�����ړ��X�V
void MovementCom::VerticalUpdate(float elapsedTime)
{
    //���E�̃X�s�[�h
    float worldSpeed = Graphics::Instance().GetWorldSpeed();
    //�I�u�W�F�N�g�̃X�s�[�h
    float objSpeed = GetGameObject()->GetObjSpeed();
    float gravity = gravity_ * (elapsedTime * worldSpeed * objSpeed);
    //float gravity = gravity_ * (elapsedTime * Graphics::Instance().GetFPS());
    //float gravity = gravity_ * (elapsedTime*100 /** Graphics::Instance().GetFPS()*/);
    AddForce({ 0,gravity,0 });
    //AddNonMaxSpeedForce({ 0,gravity,0 });
}

//�������ړ��X�V
void MovementCom::HorizonUpdate(float elapsedTime)
{
    DirectX::XMFLOAT3 horizonVelocity = { velocity_.x,0,velocity_.z };
    DirectX::XMVECTOR HorizonVelocity = DirectX::XMLoadFloat3(&horizonVelocity);
    float horiLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(HorizonVelocity));


    //�ő呬�x�ݒ�
    if (horiLength > moveMaxSpeed_)
    {
        DirectX::XMVECTOR MaxSpeed = DirectX::XMVectorScale(DirectX::XMVector3Normalize(HorizonVelocity), moveMaxSpeed_);
        DirectX::XMFLOAT3 newMaxVelocity;
        DirectX::XMStoreFloat3(&newMaxVelocity, MaxSpeed);
        velocity_.x = newMaxVelocity.x;
        velocity_.z = newMaxVelocity.z;
    }

    //���E�̃X�s�[�h
    float worldSpeed = Graphics::Instance().GetWorldSpeed();
    //�I�u�W�F�N�g�̃X�s�[�h
    float objSpeed = GetGameObject()->GetObjSpeed();


    float friction = friction_ * (elapsedTime * worldSpeed * objSpeed);
    //float friction = friction_ * (elapsedTime * Graphics::Instance().GetFPS() * worldSpeed);
    //float friction = friction_ * (elapsedTime*100 /** Graphics::Instance().GetFPS()*/);
    //���C��
    if (horiLength > 0.1f)
    {
        DirectX::XMVECTOR FriVelocity = DirectX::XMVectorScale(DirectX::XMVectorScale(HorizonVelocity, -1), friction);
        //DirectX::XMVECTOR FriVelocity = DirectX::XMVectorScale(DirectX::XMVector3Normalize(HorizonVelocity), -friction);
        //DirectX::XMFLOAT3 newVelocity;
        //DirectX::XMStoreFloat3(&newVelocity, FriVelocity);
        DirectX::XMStoreFloat3(&velocity_, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&velocity_), FriVelocity));
    }
    else
    {
        velocity_.x = 0;
        velocity_.z = 0;
    }

    //�ő呬�x�̖������C�v�Z
    horizonVelocity = { nonMaxSpeedVelocity_.x,0,nonMaxSpeedVelocity_.z };
    HorizonVelocity = DirectX::XMLoadFloat3(&horizonVelocity);
    horiLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(HorizonVelocity));

    //���C��
    if (horiLength > friction)
    {
        DirectX::XMVECTOR FriVelocity = DirectX::XMVectorScale(DirectX::XMVectorScale(HorizonVelocity, -1), friction);
        DirectX::XMFLOAT3 newVelocity;
        DirectX::XMStoreFloat3(&newVelocity, FriVelocity);
        AddNonMaxSpeedForce(newVelocity);
    }
    else
    {
        nonMaxSpeedVelocity_.x = 0;
        nonMaxSpeedVelocity_.z = 0;
    }
}

//���͂��X�V
void MovementCom::VelocityAppPosition(float elapsedTime)
{
    DirectX::XMFLOAT3 position = GetGameObject()->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 velocity;
    velocity.x = velocity_.x + nonMaxSpeedVelocity_.x;
    velocity.y = velocity_.y + nonMaxSpeedVelocity_.y;
    velocity.z = velocity_.z + nonMaxSpeedVelocity_.z;


    //���E�̃X�s�[�h
    float worldSpeed = Graphics::Instance().GetWorldSpeed();
    //�I�u�W�F�N�g�̃X�s�[�h
    float objSpeed = GetGameObject()->GetObjSpeed();

    position.x += velocity.x * (elapsedTime * worldSpeed * objSpeed);
    position.y += velocity.y * (elapsedTime * worldSpeed * objSpeed);
    position.z += velocity.z * (elapsedTime * worldSpeed * objSpeed);


    //�Ƃ肠����0�ȉ��␳
    if (position.y <= 0)
    {
        velocity_.y = 0;
        position.y = 0;
        GetGameObject()->transform_->SetWorldPosition(position);

        nonMaxSpeedVelocity_.y = 0;
        position.y = 0;
        GetGameObject()->transform_->SetWorldPosition(position);

        onGround_ = true;
    }
    else
    {
        onGround_ = false;
    }


    GetGameObject()->transform_->SetWorldPosition(position);
}
