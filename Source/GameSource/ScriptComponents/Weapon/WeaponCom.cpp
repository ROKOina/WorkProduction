#include "WeaponCom.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\AnimationCom.h"

#include "../CharacterStatusCom.h"

// �J�n����
void WeaponCom::Start()
{
    //��]
    GetGameObject()->transform_->SetEulerRotation(DirectX::XMFLOAT3(-154, -85, 82));
    //������傫��
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.19f);
}

// �X�V����
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_);
    assert(nodeName_.size() > 0);

    onHit_ = false;

    std::shared_ptr<RendererCom> rendererCom = parentObject_->GetComponent<RendererCom>();
    Model::Node* node = rendererCom->GetModel()->FindNode(nodeName_.c_str());

    //�e�ɂ���
    GetGameObject()->transform_->SetParentTransform(node->worldTransform);


    //�C�x���g���瓖���蔻���t����
    std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
    if (CollsionFromEventJudge())
        capsule->SetEnabled(true);
    else
        capsule->SetEnabled(false);

    //�J�v�Z�������蔻��ݒ�
    if (capsule->GetEnabled())
    {
        //���悩�猕����ݒ�
        capsule->SetPosition1({ 0,0,0 });
        DirectX::XMFLOAT3 up = GetGameObject()->transform_->GetWorldUp();
        capsule->SetPosition2({ up.x,up.y,up.z });

        for (auto& coll : capsule->OnHitGameObject())
        {
            //��Ԋm�F
            std::shared_ptr<CharacterStatusCom> status = coll.gameObject->GetComponent<CharacterStatusCom>();
            if (status->GetIsInvincible())continue;

            //���݂̃A�j���[�V�����C���f�b�N�X�擾
            std::shared_ptr<AnimationCom> animCom = GetGameObject()->GetParent()->GetComponent<AnimationCom>();
            int animIndex = animCom->GetCurrentAnimationIndex();

            //��΂�����
            DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&GetGameObject()->GetParent()->transform_->GetWorldFront());
            DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&GetGameObject()->GetParent()->transform_->GetWorldUp());
            //�����̊���
            F = DirectX::XMVectorScale(F, attackStatus_[animIndex].front);
            U = DirectX::XMVectorScale(U, attackStatus_[animIndex].up);
            //����
            DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorAdd(F, U));
            DirectX::XMFLOAT3 dir;
            DirectX::XMStoreFloat3(&dir, Dir);

            //������΂�
            float power = attackStatus_[animIndex].impactPower;
            status->OnDamage(DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power ));

            onHit_ = true;
        }
    }
}

// GUI�`��
void WeaponCom::OnGUI()
{

}


void WeaponCom::SetAttackStatus(int animIndex, int damage, float impactPower, float front, float up)
{
    attackStatus_[animIndex].damage = damage;
    attackStatus_[animIndex].impactPower = impactPower;
    attackStatus_[animIndex].front = front;
    attackStatus_[animIndex].up = up;
}


//�A�j���C�x���g�����瓖���蔻���t���邩���f("AutoCollision"����n�܂�C�x���g�������Ŏ擾)
bool WeaponCom::CollsionFromEventJudge()
{
    std::shared_ptr<AnimationCom> animCom = GetGameObject()->GetParent()->GetComponent<AnimationCom>();
    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //����AutoCollision�Ȃ瓖���蔻�������
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;
        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3()))continue;

        return true;
    }
    return false;
}
