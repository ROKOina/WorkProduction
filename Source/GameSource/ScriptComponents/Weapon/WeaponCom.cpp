#include "WeaponCom.h"

#include <imgui.h>

#include "Components\TransformCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\ParticleSystemCom.h"

#include "../CharacterStatusCom.h"

// �J�n����
void WeaponCom::Start()
{
}

// �X�V����
void WeaponCom::Update(float elapsedTime)
{
    assert(parentObject_.lock());

    //�I�����o
    DirectionEnd(elapsedTime);

    //���݂̃A�j���[�V�����C���f�b�N�X�擾
    std::shared_ptr<AnimationCom> animCom = parentObject_.lock()->GetComponent<AnimationCom>();
    //���̃A�j���[�V�������o�^����Ă��邩�m�F
    auto it = attackStatus_.find(animCom->GetCurrentAnimationIndex());
    if (it == attackStatus_.end() && !isForeverUse_)    //�o�^����Ȃ���
    {
        if (isWeaponUse) //����g�p�I����
        {
            isWeaponUse = false; 

            //�U���I������
            isAttackAnim_ = false;

            //�I�����o�N��
            isDirectionEnd_ = true;
            directionState_ = 0;

            //�R���C�_�[�؂�
            std::shared_ptr<Collider> col = GetGameObject()->GetComponent<Collider>();
            col->SetEnabled(false);

        }
        return;
    }
    else
    {
        if (!isWeaponUse)
        {
            isWeaponUse = true;

            //���퉉�o�N��
            isDirectionStart_ = true;
            directionState_ = 0;

            //�p�[�e�B�N���o��
            if (GetGameObject()->GetChildren().size() > 0)
            {
                std::shared_ptr<GameObject> particle = GetGameObject()->GetChildren()[0].lock();
                if (particle->GetComponent<ParticleSystemCom>())
                    particle->GetComponent<ParticleSystemCom>()->SetRoop(true);
            }
        }
    }

    //�o�����o
    DirectionStart(elapsedTime);

    //�q�b�g�m�F���Z�b�g
    onHit_ = false;

    //�e�q�֌W�X�V
    ParentSetting();

    //�R���W�����ŏ�����ς���U������
    CollisionWeaponAttack();
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

void WeaponCom::SetAttackDefaultStatus(int damage, float impactPower, float front, float up, float animSpeed, ATTACK_SPECIAL_TYPE specialAttack)
{
    defaultStatus_.damage = damage;
    defaultStatus_.impactPower = impactPower;
    defaultStatus_.front = front;
    defaultStatus_.up = up;
    defaultStatus_.specialType = specialAttack;
}

void WeaponCom::CollisionWeaponAttack()
{
    //�U���A�j���Ȃ�true
    bool isAttackCollision = CollsionFromEventJudge();

    std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
    if (capsule)    //�J�v�Z�������蔻��̏ꍇ
    {
        //�C�x���g���瓖���蔻���t����
        if (isAttackCollision)
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
                //�U������
                AttackProcess(coll.gameObject.lock());
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

        return;
    }

    std::shared_ptr<SphereColliderCom> sphere = GetGameObject()->GetComponent<SphereColliderCom>();
    if (sphere) //�������蔻��̏ꍇ
    {
        for (auto& coll : sphere->OnHitGameObject())
        {
            //�U������
            AttackProcess(coll.gameObject.lock(), false, 0.2f);
        }

        return;
    }
}

void WeaponCom::ParentSetting()
{
    assert(nodeParent_.lock());


    nodeParent_.lock()->UpdateTransform();
    DirectX::XMFLOAT4X4 nodeTrasform;

    //�m�[�h���O�̗L���Ŕ���
    if (nodeName_.size() > 0)
    {
        std::shared_ptr<RendererCom> parentRendererCom = nodeParent_.lock()->GetComponent<RendererCom>();
        nodeTrasform = parentRendererCom->GetModel()->FindNode(nodeName_.c_str())->worldTransform;
    }
    else
        //return;
        nodeTrasform = nodeParent_.lock()->transform_->GetWorldTransform();

    //�e�ɂ���
    GetGameObject()->transform_->SetParentTransform(nodeTrasform);
}

void WeaponCom::AttackProcess(std::shared_ptr<GameObject> damageObj, bool useAnim, float invincibleTime)
{
    //��Ԋm�F
    std::shared_ptr<CharacterStatusCom> status = damageObj->GetComponent<CharacterStatusCom>();
    if (status->GetIsInvincible())return;

    //�A�j���[�V�������g�p����Ƃ�
    if (useAnim)
    {
        //���݂̃A�j���[�V�����C���f�b�N�X�擾
        std::shared_ptr<AnimationCom> animCom = parentObject_.lock()->GetComponent<AnimationCom>();
        int animIndex = animCom->GetCurrentAnimationIndex();

        //��΂�����
        DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldFront());
        DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldUp());
        //�����̊���
        F = DirectX::XMVectorScale(F, attackStatus_[animIndex].front);
        U = DirectX::XMVectorScale(U, attackStatus_[animIndex].up);
        //����
        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorAdd(F, U));
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //������΂�
        float power = attackStatus_[animIndex].impactPower;
        status->OnDamage(GetGameObject()->GetComponent<WeaponCom>(), attackStatus_[animIndex].damage, DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power)
            , attackStatus_[animIndex].specialType, invincibleTime);
    }
    else
    {
        //��΂�����
        DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldFront());
        DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&parentObject_.lock()->transform_->GetWorldUp());
        //�����̊���
        F = DirectX::XMVectorScale(F, defaultStatus_.front);
        U = DirectX::XMVectorScale(U, defaultStatus_.up);
        //����
        DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(DirectX::XMVectorAdd(F, U));
        DirectX::XMFLOAT3 dir;
        DirectX::XMStoreFloat3(&dir, Dir);

        //������΂�
        float power = defaultStatus_.impactPower;
        status->OnDamage(GetGameObject()->GetComponent<WeaponCom>(),defaultStatus_.damage, DirectX::XMFLOAT3(dir.x * power, dir.y * power, dir.z * power)
            , defaultStatus_.specialType, invincibleTime);
    }

    onHit_ = true;
}

//�A�j���C�x���g�����瓖���蔻���t���邩���f("AutoCollision"����n�܂�C�x���g�������Ŏ擾)
bool WeaponCom::CollsionFromEventJudge()
{
    std::shared_ptr<AnimationCom> animCom = parentObject_.lock()->GetComponent<AnimationCom>();

    //�A�j���[�V�������x�ύX���Ă�����߂�
    if (isAnimSetting)
    {
        //�U�����x��������
        std::shared_ptr<AnimatorCom> animator = parentObject_.lock()->GetComponent<AnimatorCom>();
        animator->SetAnimationSpeedOffset(1);
        isAnimSetting = false;
    }

    //���̃A�j���[�V�������o�^����Ă��邩�m�F
    auto it = attackStatus_.find(animCom->GetCurrentAnimationIndex());
    if (it == attackStatus_.end())    //�o�^����Ȃ���
        return false;

    for (auto& animEvent : animCom->GetCurrentAnimationEventsData())
    {

        //����AutoCollision�Ȃ瓖���蔻�������
        if (animEvent.name.compare(0, 13, "AutoCollision") != 0)continue;

        //�U���A�j���[�V��������true
        isAttackAnim_ = true;
        //���݂̃A�j���[�V�����ۑ�
        attackAnimIndex_ = parentObject_.lock()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

        //�U�����x��������
        std::shared_ptr<AnimatorCom> animator = parentObject_.lock()->GetComponent<AnimatorCom>();

        //�G���h�t���[���O�Ȃ�
        if (!animCom->GetCurrentAnimationEventIsEnd(animEvent.name.c_str()))
        {
            //�A�j���[�V�����X�s�[�h��ݒ�
            animator->SetAnimationSpeedOffset(attackStatus_[animCom->GetCurrentAnimationIndex()].animSpeed);
            isAnimSetting = true;
        }

        if (!animCom->GetCurrentAnimationEvent(animEvent.name.c_str(), DirectX::XMFLOAT3(0, 0, 0)))continue;

        return true;
    }
    return false;
}

void WeaponCom::DirectionStart(float elapsedTime)
{
    if (!isDirectionStart_)return;

    std::shared_ptr<RendererCom> renderCom = GetGameObject()->GetComponent<RendererCom>();
    if (!renderCom) //�`�悷����̂��Ȃ��ꍇ
    {
        //���o�I��
        isDirectionStart_ = false;
        directionState_ = -1;
    }

    //���点�鏈��
    switch (directionState_)
    {
    case 0:
    {
        renderCom->SetEnabled(true);
        renderCom->GetModel()->SetMaterialColor({ 1,2,1,2 });

        directionState_++;
    }
    break;
    case 1:
    {
        DirectX::XMFLOAT4 color = renderCom->GetModel()->GetMaterialColor();
        color.y -= 2 * elapsedTime;
        color.w -= 2 * elapsedTime;
        if (color.y < 1 && color.w < 1)
        {
            //���o�I��
            isDirectionStart_ = false;
            directionState_ = -1;
        }
        if (color.y < 1)color.y = 1;
        if (color.w < 1)color.w = 1;
        renderCom->GetModel()->SetMaterialColor(color);
    }
    break;
    }

}

void WeaponCom::DirectionEnd(float elapsedTime)
{
    if (!isDirectionEnd_)return;

    std::shared_ptr<RendererCom> renderCom = GetGameObject()->GetComponent<RendererCom>();
    if (!renderCom) //�`�悷����̂��Ȃ��ꍇ
    {
        //���o�I��
        isDirectionEnd_ = false;
        directionState_ = -1;

        //�p�[�e�B�N���؂�
        if (GetGameObject()->GetChildren().size() > 0)
        {
            std::shared_ptr<GameObject> particle = GetGameObject()->GetChildren()[0].lock();
            if (particle->GetComponent<ParticleSystemCom>())
                particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
        }
    }

    switch (directionState_)
    {
    case 0:
    {
        DirectX::XMFLOAT4 color = renderCom->GetModel()->GetMaterialColor();
        color.y += 2 * elapsedTime;
        color.w += 2 * elapsedTime;
        if (color.y > 2 && color.w > 2)
        {
            //���o�I��
            renderCom->SetEnabled(false);

            isDirectionEnd_ = false;
            directionState_ = -1;

            //�p�[�e�B�N���؂�
            if (GetGameObject()->GetChildren().size() > 0)
            {
                std::shared_ptr<GameObject> particle = GetGameObject()->GetChildren()[0].lock();
                if (particle->GetComponent<ParticleSystemCom>())
                    particle->GetComponent<ParticleSystemCom>()->SetRoop(false);
            }

        }
        renderCom->GetModel()->SetMaterialColor(color);
    }
    break;
    }
}
