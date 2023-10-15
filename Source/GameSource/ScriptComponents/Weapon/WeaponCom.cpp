#include "WeaponCom.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"

#include "../CharacterStatusCom.h"

// �J�n����
void WeaponCom::Start()
{
    ////��]
    //GetGameObject()->transform_->SetEulerRotation(DirectX::XMFLOAT3(-154, -85, 82));
    ////������傫��
    //GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.19f);
}

// �X�V����
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_.lock());
    assert(nodeName_.size() > 0);

    //�q�b�g�m�F���Z�b�g
    onHit_ = false;

    parentObject_.lock()->UpdateTransform();
    std::shared_ptr<RendererCom> parentRendererCom = parentObject_.lock()->GetComponent<RendererCom>();
    Model::Node* parentNode = parentRendererCom->GetModel()->FindNode(nodeName_.c_str());

    //�e�ɂ���
    GetGameObject()->transform_->SetParentTransform(parentNode->worldTransform);

    std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
    //�C�x���g���瓖���蔻���t����
    if (CollsionFromEventJudge())
        capsule->SetEnabled(true);
    else
        capsule->SetEnabled(false);

    //�J�v�Z�������蔻��ݒ�
    if (capsule->GetEnabled())
    {
        //���悩�猕����ݒ�    
        DirectX::XMFLOAT3 up = GetGameObject()->transform_->GetWorldUp();
        capsule->SetPosition1({
             up.x * colliderUpDown_.y
            ,up.y * colliderUpDown_.y
            ,up.z * colliderUpDown_.y
            });
        capsule->SetPosition2({
             up.x * colliderUpDown_.x
            ,up.y * colliderUpDown_.x
            ,up.z * colliderUpDown_.x
            });

        for (auto& coll : capsule->OnHitGameObject())
        {
            //��Ԋm�F
            std::shared_ptr<CharacterStatusCom> status = coll.gameObject.lock()->GetComponent<CharacterStatusCom>();
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
            status->OnDamage(DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power)
                , attackStatus_[animIndex].specialType);

            onHit_ = true;
        }
    }

    //�U���A�j���[�V��������
    if (isAttackAnim_)
    {
        if (oldIsAnim_)
            if (attackAnimIndex_ != oldAnimIndex)
                isAttackAnim_ = false;  //�U���R���{�̏ꍇ�A�P�t���[������false��

        oldAnimIndex = parentObject_.lock()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();
    }
    oldIsAnim_ = isAttackAnim_;

}

// GUI�`��
void WeaponCom::OnGUI()
{
    ImGui::DragFloat2("colliderUpDown", &colliderUpDown_.x, 0.01f);
}


void WeaponCom::SetAttackStatus(int animIndex, int damage, float impactPower, float front, float up, float animSpeed, ATTACK_SPECIAL_TYPE specialAttack)
{
    attackStatus_[animIndex].damage = damage;
    attackStatus_[animIndex].impactPower = impactPower;
    attackStatus_[animIndex].front = front;
    attackStatus_[animIndex].up = up;
    attackStatus_[animIndex].specialType = specialAttack;

    //�A�j���[�V�����X�s�[�h
    attackStatus_[animIndex].animSpeed = animSpeed;
}


//�A�j���C�x���g�����瓖���蔻���t���邩���f("AutoCollision"����n�܂�C�x���g�������Ŏ擾)
bool WeaponCom::CollsionFromEventJudge()
{
    std::shared_ptr<AnimationCom> animCom = GetGameObject()->GetParent()->GetComponent<AnimationCom>();

    //�U�����x��������
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetParent()->GetComponent<AnimatorCom>();

    //�A�j���[�V�������x�ύX���Ă�����߂�
    if (isAnimSetting)
    {
        animator->SetAnimationSpeedOffset(1);
        isAnimSetting = false;
    }

    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {
        //����AutoCollision�Ȃ瓖���蔻�������
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;

        //�U���A�j���[�V��������true
        isAttackAnim_ = true;
        //���݂̃A�j���[�V�����ۑ�
        attackAnimIndex_ = parentObject_.lock()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

        //�G���h�t���[���O�Ȃ�
        if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))
        {
            //�A�j���[�V�����X�s�[�h��ݒ�
            animator->SetAnimationSpeedOffset(attackStatus_[animCom->GetCurrentAnimationIndex()].animSpeed);
            isAnimSetting = true;
        }

        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3()))continue;

        return true;
    }
    return false;
}
