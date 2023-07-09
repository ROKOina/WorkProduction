#include "ColliderCom.h"
#include "Graphics\Graphics.h"
#include "Components\TransformCom.h"
#include <imgui.h>

//�����蔻��
#pragma region Collider

//����O�̃N���A
void Collider::ColliderStartClear()
{
    hitGameObject_.clear();
}

//������w�肵�Ĕ���
void Collider::ColliderVSOther(std::shared_ptr<Collider> otherSide)
{
    //�^�O�Ŕ��肷�邩���߂�
    if ((myTag_ == NONE) || (otherSide->myTag_ == NONE))return;
    if ((judgeTag_ != otherSide->myTag_) && (myTag_ != otherSide->judgeTag_))return;

    //�`�󖈂ɓ����蔻���ς���
    int myType = colliderType_;
    int otherType = otherSide->colliderType_;

    bool isJudge = false;

    //��
    if (myType == COLLIDER_TYPE::SphereCollider) {
        if (otherType == COLLIDER_TYPE::SphereCollider)//vs��
            isJudge = SphereVsSphere(otherSide);
        else if (otherType == COLLIDER_TYPE::BoxCollider)//vs�{�b�N�X
            isJudge = SphereVsBox(otherSide);
    }
    //�{�b�N�X
    else if(myType == COLLIDER_TYPE::BoxCollider)
    {
        if (otherType == COLLIDER_TYPE::BoxCollider)//vs�{�b�N�X
            isJudge = BoxVsBox(otherSide);
        else if (otherType == COLLIDER_TYPE::SphereCollider)//vs��
            isJudge = SphereVsBox(otherSide);
    }

    //�����������̓Q�[���I�u�W�F�N�g�ŕۑ�
    if (isJudge)
    {
        if (judgeTag_ == otherSide->myTag_)
            hitGameObject_.emplace_back(otherSide->GetGameObject());

        if (otherSide->judgeTag_ == myTag_)
            otherSide->hitGameObject_.emplace_back(GetGameObject());
    }
}


//��v��
bool Collider::SphereVsSphere(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<SphereColliderCom> mySphere = std::static_pointer_cast<SphereColliderCom>(shared_from_this());
    std::shared_ptr<SphereColliderCom> otherSphere = std::static_pointer_cast<SphereColliderCom>(otherSide);

    //�K�v�ȃp�����[�^�[�擾
    DirectX::XMFLOAT3 myPos = mySphere->GetGameObject()->transform_->GetPosition();
    myPos = { myPos.x + offsetPos_.x,myPos.y + offsetPos_.y,myPos.z + offsetPos_.z };
    DirectX::XMFLOAT3 otherPos = otherSphere->GetGameObject()->transform_->GetPosition();
    otherPos = { otherPos.x + otherSphere->offsetPos_.x,otherPos.y + otherSphere->offsetPos_.y,otherPos.z + otherSphere->offsetPos_.z };
    DirectX::XMVECTOR MyPos = DirectX::XMLoadFloat3(&myPos);
    DirectX::XMVECTOR OtherPos = DirectX::XMLoadFloat3(&otherPos);
    float myRadius = mySphere->GetRadius();
    float otherRadius = otherSphere->GetRadius();

    //�������߂�
    float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(MyPos, OtherPos)));
    float radius = myRadius + otherRadius;

    //����
    if (dist < radius)
        return true;

    return false;
}

//��v��
bool Collider::BoxVsBox(std::shared_ptr<Collider> otherSide)
{
    std::shared_ptr<BoxColliderCom> myBox = std::static_pointer_cast<BoxColliderCom>(shared_from_this());
    std::shared_ptr<BoxColliderCom> otherBox = std::static_pointer_cast<BoxColliderCom>(otherSide);

    //�K�v�ȃp�����[�^�[�擾
    DirectX::XMFLOAT3 myPos = myBox->GetGameObject()->transform_->GetPosition();
    myPos = { myPos.x + offsetPos_.x,myPos.y + offsetPos_.y,myPos.z + offsetPos_.z };
    DirectX::XMFLOAT3 otherPos = otherBox->GetGameObject()->transform_->GetPosition();
    otherPos = { otherPos.x + otherBox->offsetPos_.x,otherPos.y + otherBox->offsetPos_.y,otherPos.z + otherBox->offsetPos_.z };
    DirectX::XMFLOAT3 mySize = myBox->GetSize();
    DirectX::XMFLOAT3 otherSize = otherBox->GetSize();

    //Y��
    float distY = myPos.y - otherPos.y;
    if (distY * distY > (mySize.y + otherSize.y) * (mySize.y + otherSize.y))
        return false;
    
    //X��
    float distX = myPos.x - otherPos.x;
    if (distX * distX > (mySize.x + otherSize.x) * (mySize.x + otherSize.x))
        return false;

    //Z��
    float distZ = myPos.z - otherPos.z;
    if (distZ * distZ > (mySize.z + otherSize.z) * (mySize.z + otherSize.z))
        return false;

    return true;
}

//��v��
bool Collider::SphereVsBox(std::shared_ptr<Collider> otherSide)
{
    //�`��𔻒�
    std::shared_ptr<SphereColliderCom> sphere;
    std::shared_ptr<BoxColliderCom> box;

    if (colliderType_ == COLLIDER_TYPE::SphereCollider)
    {
        sphere = std::static_pointer_cast<SphereColliderCom>(shared_from_this());
        box = std::static_pointer_cast<BoxColliderCom>(otherSide);
    }
    else
    {
        sphere = std::static_pointer_cast<SphereColliderCom>(otherSide);
        box = std::static_pointer_cast<BoxColliderCom>(shared_from_this());
    }

    //�{�b�N�X�Ɠ_�̍ŒZ�������擾
    auto PointToBoxLengthSq = [](float boxSize, float point)
    {
        float SqLen = 0;   // �����ׂ̂���̒l���i�[
        if (point < -boxSize)
            SqLen += (point + boxSize) * (point + boxSize);
        if (point > boxSize)
            SqLen += (point - boxSize) * (point - boxSize);
        return SqLen;
    };

    //���W�擾
    DirectX::XMFLOAT3 spherePos = sphere->GetGameObject()->transform_->GetPosition();
    DirectX::XMFLOAT3 boxPos = box->GetGameObject()->transform_->GetPosition();
    //�{�b�N�X����Ƃ������W�ɂ���
    DirectX::XMFLOAT3 spherePosFromBox = { spherePos.x - boxPos.x,spherePos.y - boxPos.y,spherePos.z - boxPos.z };

    //���̒��S�_�ƃ{�b�N�X�̍ŒZ����
    float nearLengthSq = 0;
    nearLengthSq += PointToBoxLengthSq(box->GetSize().x, spherePosFromBox.x);
    nearLengthSq += PointToBoxLengthSq(box->GetSize().y, spherePosFromBox.y);
    nearLengthSq += PointToBoxLengthSq(box->GetSize().z, spherePosFromBox.z);

    //�ŒZ�_�����̔��a�ȉ��Ȃ瓖�����Ă���
    if (nearLengthSq < sphere->GetRadius() * sphere->GetRadius())
        return true;

    return false;
}


#pragma endregion


/// ������`��֐� ///

//��
#pragma region SphereCollider

// GUI�`��
void SphereColliderCom::OnGUI()
{
    ImGui::DragFloat("radius", &radius_,0.1f);
    ImGui::DragFloat3("offsetPos", &offsetPos_.x,0.1f);
}

// debug�`��
void SphereColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetPosition();
    pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
    Graphics::Instance().GetDebugRenderer()->DrawSphere(
       pos, radius_, { 1,0,0,1 });
}

#pragma endregion

//�l�p
#pragma region BoxCollider

// GUI�`��
void BoxColliderCom::OnGUI()
{
    ImGui::DragFloat3("size", &size_.x, 0.1f);
    ImGui::DragFloat3("offsetPos", &offsetPos_.x, 0.1f);
}

// debug�`��
void BoxColliderCom::DebugRender()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetPosition();
    pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
    Graphics::Instance().GetDebugRenderer()->DrawBox(pos, size_, { 1,0,0,1 });
}

#pragma endregion