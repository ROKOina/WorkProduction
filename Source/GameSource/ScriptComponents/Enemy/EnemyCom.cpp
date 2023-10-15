#include "EnemyCom.h"
#include "Components\AnimationCom.h"
#include "Components\TransformCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"
#include "GameSource/Math/Mathf.h"

#include "../Player/PlayerCom.h"
#include "../CharacterStatusCom.h"

#include <imgui.h>


// �J�n����
void EnemyCom::Start()
{

}

// �X�V����
void EnemyCom::Update(float elapsedTime)
{
    //�����オ�胂�[�V��������
    StandUpUpdate();

    // ���ݎ��s����Ă���m�[�h���������
    if (activeNode_ == nullptr&& !isAnimDamage_&& !isJumpDamage_&& !isStandUpMotion_)
    {
        // ���Ɏ��s����m�[�h�𐄘_����B
        activeNode_ = aiTree_->ActiveNodeInference(behaviorData_);
    }
    // ���ݎ��s����m�[�h�������
    if (activeNode_ != nullptr)
    {
         //�r�w�C�r�A�c���[����m�[�h�����s�B
        std::shared_ptr<NodeBase> n = aiTree_->Run(activeNode_, behaviorData_, elapsedTime);
        if (!n)
        {
            //����
            activeNode_.reset();
        }
        else
        {
            if (n->GetId() != activeNode_->GetId())
                activeNode_.swap(n);
        }
    }

    //�d�͐ݒ�
    GravityProcess(elapsedTime);

    //�_���[�W����
    DamageProcess(elapsedTime);

    //�W���X�g���p����o��
    justColliderProcess();

}

// GUI�`��
void EnemyCom::OnGUI()
{
    if (ImGui::Button("delete"))
    {
        GameObjectManager::Instance().Remove(GetGameObject());
    }

    ImGui::DragFloat("walkSpeed", &moveDataEnemy.walkSpeed);
    ImGui::DragFloat("walkMaxSpeed", &moveDataEnemy.walkMaxSpeed);
    ImGui::DragFloat("runSpeed", &moveDataEnemy.runSpeed);
    ImGui::DragFloat("runMaxSpeed", &moveDataEnemy.runMaxSpeed);
}

// �^�[�Q�b�g�ʒu�������_���ݒ�
void EnemyCom::SetRandomTargetPosition()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos.x += Mathf::RandomRange(-3, 3);
    pos.z += Mathf::RandomRange(-3, 3);
    targetPosition_ = pos;
}

//�v���C���[���߂���true
bool EnemyCom::SearchPlayer()
{
    DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();

    float vx = playerPos.x - pos.x;
    float vy = playerPos.y - pos.y;
    float vz = playerPos.z - pos.z;
    float dist = sqrtf(vx * vx + vy * vy + vz * vz);

    if (dist < searchRange_)
    {
        return true;
    }
    return false;
}

//�_���[�W�m�F
bool EnemyCom::OnDamageEnemy()
{
    return GetGameObject()->GetComponent<CharacterStatusCom>()->GetFrameDamage();
}

//��e�[�������オ�胂�[�V����
void EnemyCom::SetStandUpMotion()
{
    //���n�A�j���[�V���������
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetTriggerOn("damageFallEnd");
    isStandUpMotion_ = true;
    playStandUp_ = false;
    //�U�����󂯂Ă��ړ����Ȃ��悤��
    GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(true);
}
void EnemyCom::StandUpUpdate()
{
    if (isStandUpMotion_)
    {
        std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
        int index = animation->GetCurrentAnimationIndex();
        if (index == FALL_STAND_UP) //�N���オ�胂�[�V������
        {
            playStandUp_ = true;
        }
        if (playStandUp_)
        {
            if (!animation->IsPlayAnimation())
            {
                playStandUp_ = false;
                isStandUpMotion_ = false;
                GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(false);
            }
        }
    }
}

//�_���[�W����
void EnemyCom::DamageProcess(float elapsedTime)
{
    //�󒆃_���[�W���d�͂������̊�0�ɂ���
    {
        if (!isAnimDamage_ && oldAnimDamage_)
        {
            if (!GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                skyGravityZeroTimer_ = 0.5f;
            }
        }
        oldAnimDamage_ = isAnimDamage_;
    }

    //�_���[�W�A�j���[�V��������
    if (isAnimDamage_)
    {
        isAnimDamage_ = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageAnimation();
    }

    //�W�����v��e���̒��n����
    if (isJumpDamage_)
    {
        if (!isAnimDamage_)
        {
            if (GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                isJumpDamage_ = false;
                //�����オ�胂�[�V�����N��
                SetStandUpMotion();
            }
        }
    }

    //�_���[�W����
    if (OnDamageEnemy())
    {
        bool endTree = false;   //���̑J�ڂ��I��点�Ĕ�e�A�j���[�V��������Ȃ�true

        //�A���X�g�b�p�u����e�Ȃ炻�̂܂܃A�j���[�V����
        ATTACK_SPECIAL_TYPE attackType = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageType();
        if (attackType == ATTACK_SPECIAL_TYPE::UNSTOP)
            endTree = true;
        //�W�����v���͖��h��
        if (attackType == ATTACK_SPECIAL_TYPE::JUMP_NOW)
            endTree = true;

        //�A�N�V�����m�[�h���Ȃ��ꍇ
        if (!activeNode_)
            endTree = true;

        //��e����s�������f
        for (int& id : damageAnimAiTreeId_)
        {
            if (endTree)break;

            NodeBase* node = activeNode_.get();
            //��e���ɃA�j���[�V�������邩�m�F
            while (1)
            {
                //���Ȃ�ID�Ȃ�endTree��true��
                if (node->GetId() == id)
                {
                    endTree = true;
                    break;
                }

                //�e���m�F����
                node = node->GetParent().get();
                if (!node)break;
            }

            //�J�ڂ��I��点�ăA�j���[�V��������
            if (endTree)
            {
                activeNode_->EndActionSetStep();
                activeNode_->Run(GetGameObject()->GetComponent<EnemyCom>(), elapsedTime);
                //����
                activeNode_.reset();
            }
        }

        //�_���[�W����
        if (endTree)
        {
            isAnimDamage_ = true;
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
            animator->ResetParameterList();

            //�A�j���[�V�����̎�ނ𔻒�
            if (isStandUpMotion_)   //�����オ�胂�[�V��������D��
            {
                SetStandUpMotion();
            }
            else if (attackType == ATTACK_SPECIAL_TYPE::JUMP_START)  //�؂�グ��e��
            {
                animator->SetTriggerOn("damageGoFly");
                isJumpDamage_ = true;

            }
            else if (isJumpDamage_)    //�󒆔�e��
            {
                animator->SetTriggerOn("damageInAir");
            }
            else
            {
                animator->SetTriggerOn("damage");
            }
        }
    }
}

//�W���X�g���p����o��
void EnemyCom::justColliderProcess()
{
    //�W���X�g��𓖂��蔻���؂�A�A�^�b�N�����蔻������Ă���
    DirectX::XMFLOAT3 pos;
    //�W���X�g
    std::shared_ptr<GameObject> justChild = GetGameObject()->GetChildFind("picolaboAttackJust");

    //�W���X�g�����蔻���؂��Ă���
    justChild->GetComponent<Collider>()->SetEnabled(false);

    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    for (auto& animEvent : animation->GetCurrentAnimationEventsData())
    {
        //just�������Ă���Ȃ�Ȃ�
        if (animEvent.name.find("just") == std::string::npos)continue;
        if (isJustAvoid_)justChild->GetComponent<Collider>()->SetEnabled(true);
        //�C�x���g���Ȃ瓖���蔻����o��
        if (animation->GetCurrentAnimationEvent(animEvent.name.c_str(), pos))
        {
            isJustAvoid_ = false;
            justChild->GetComponent<Collider>()->SetEnabled(true);
        }
    }
}

//�d�͐ݒ�
void EnemyCom::GravityProcess(float elapsedTime)
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();

    //�A�j���[�V�����C�x���g�����ďd�͂��Ȃ���
    if (animation->GetCurrentAnimationEvent("ZeroGravity", DirectX::XMFLOAT3()) || skyGravityZeroTimer_ > 0)
    {
        skyGravityZeroTimer_ -= elapsedTime;
        isSetGravity_ = true;
        GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
        GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
    }
    else
    {
        if (isSetGravity_)   //�d�͐ݒ����������������
        {
            isSetGravity_ = false;
            GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
            GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
        }
    }
}

bool EnemyCom::OnMessage(const Telegram& msg)
{
    return false;
}





//��e���ɃA�j���[�V�������鎞��AITREE�����߂�
template<typename... Args>
void EnemyCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}
