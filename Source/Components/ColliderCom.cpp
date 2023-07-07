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
    //�`�󖈂ɓ����蔻���ς���
    int myType = colliderType_;
    int otherType = otherSide->colliderType_;

    //��
    if (myType == COLLIDER_TYPE::SphereCollider) {
        if (otherType == COLLIDER_TYPE::SphereCollider)
            SphereVsSphere(otherSide);
        if (otherType == COLLIDER_TYPE::BoxCollider)
            int i = 0;
    }
    //�{�b�N�X
    else if(myType == COLLIDER_TYPE::BoxCollider)
    {

    }
}


//��v��(�������Ă�����true)
bool Collider::SphereVsSphere(std::shared_ptr<Collider> otherSide)
{
    //�^�O�Ŕ��肷�邩���߂�
    if (myTag_ == NONE || judgeTag_ == NONE)return false;
    if (judgeTag_ != otherSide->myTag_)return false;

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
    {
        ////�y�����������Ԃ�����
        //std::shared_ptr<GameObject> heavyGameObject;    //�d��
        //std::shared_ptr<GameObject> lightGameObject;    //�y��

        //if (mySphere->GetWeight() > otherSphere->GetWeight())
        //{
        //    heavyGameObject = mySphere->GetGameObject();
        //    lightGameObject = otherSphere->GetGameObject();
        //}
        //else
        //{
        //    heavyGameObject = otherSphere->GetGameObject();
        //    lightGameObject = mySphere->GetGameObject();
        //}

        ////�|�W�V����
        //DirectX::XMVECTOR 

        ////�����Ԃ�����
        //float returnPush = dist - radius + 0.01f;   //�I�t�Z�b�g�ŏ�������

        ////�y�����ւ̃x�N�g��
        //DirectX::XMVectorSubtract(MyPos, OtherPos)


        //�����������̓Q�[���I�u�W�F�N�g�ŕۑ�
        hitGameObject_.emplace_back(otherSide->GetGameObject());
        otherSide->hitGameObject_.emplace_back(GetGameObject());

        return true;
    }

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

}

// debug�`��
void BoxColliderCom::DebugRender()
{
    //DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetPosition();
    //pos = { pos.x + offsetPos_.x,pos.y + offsetPos_.y,pos.z + offsetPos_.z };
    //Graphics::Instance().GetDebugRenderer()->DrawSphere
}

#pragma endregion