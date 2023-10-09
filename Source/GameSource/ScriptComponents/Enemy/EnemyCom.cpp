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
    //�_���[�W�A�j���[�V����������m�[�h��o�^
    OnDamageAnimAI_TREE(AI_TREE::WANDER, AI_TREE::PURSUIT/*, AI_TREE::NORMAL*/);
    
    //�A�j���[�V����������
    AnimationInitialize();
    // �r�w�C�r�A�c���[�ݒ�
    behaviorData_ = std::make_unique<BehaviorData>();
    aiTree_ = std::make_unique<BehaviorTree>(this);
    // BehaviorTree�}�����BehaviorTree���\�z
    aiTree_->AddNode(AI_TREE::NONE, AI_TREE::ROOT, 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //2�w
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::BATTLE, 1, BehaviorTree::SelectRule::Priority, new BattleJudgment(this), nullptr);
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::SCOUT, 2, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //3�w
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::WANDER, 1, BehaviorTree::SelectRule::Non, new WanderJudgment(this), new WanderAction(this));
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::IDLE, 2, BehaviorTree::SelectRule::Non, nullptr, new IdleAction(this));

    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ATTACK, 1, BehaviorTree::SelectRule::Random, new AttackJudgment(this), nullptr);
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::PURSUIT, 2, BehaviorTree::SelectRule::Non, nullptr, new PursuitAction(this));

    //4�w
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 1, BehaviorTree::SelectRule::Priority, new AttackJudgment(this), new AttackAction(this));

    SetRandomTargetPosition();

    //����������
    std::vector<ModelResource::Material>& materials = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared()->GetMaterialsEdit();
    materials[0].toonStruct._Emissive_Color.w = 0;

    GetGameObject()->GetComponent<CharacterStatusCom>()->SetInvincibleTime(0);
}

// �X�V����
void EnemyCom::Update(float elapsedTime)
{
    //�_���[�W�A�j���[�V��������
    {
        if (isAnimDamage_)
        {
            isAnimDamage_ = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageAnimation();
        }
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

    //�����オ�胂�[�V��������
    StandUpUpdate();

    // ���ݎ��s����Ă���m�[�h���������
    if (activeNode_ == nullptr&& !isAnimDamage_&& !isJumpDamage_&& !isStandUpMotion_)
    {
        // ���Ɏ��s����m�[�h�𐄘_����B
        activeNode_.reset(aiTree_->ActiveNodeInference(behaviorData_.get()));
    }
    // ���ݎ��s����m�[�h�������
    if (activeNode_ != nullptr)
    {
         //�r�w�C�r�A�c���[����m�[�h�����s�B
        NodeBase* n = aiTree_->Run(activeNode_.get(), behaviorData_.get(), elapsedTime);
        if (!n)
        {
            //����
            activeNode_.release();
        }
        else
        {
            if (n->GetId() != activeNode_->GetId())
                activeNode_.reset(n);
        }
    }

    //�d�͐ݒ�
    {
        std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();

        static bool isSetGravity = false;
        //�A�j���[�V�����C�x���g�����ďd�͂��Ȃ���
        if(animation->GetCurrentAnimationEvent("ZeroGravity", DirectX::XMFLOAT3()))
        {
            isSetGravity = true;
            GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
        }
        else
        {
            if (isSetGravity)   //�d�͐ݒ����������������
            {
                isSetGravity = false;
                GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
                GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
            }
        }
    }

    //�_���[�W����
    if (OnDamageEnemy())
    {
        bool endTree = false;   //���̑J�ڂ��I��点�邩

        //�A���X�g�b�p�u����e�Ȃ炻�̂܂܃A�j���[�V����
        ATTACK_SPECIAL_TYPE attackType = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageType();
        if (attackType == ATTACK_SPECIAL_TYPE::UNSTOP)
            endTree = true;
        //�W�����v���͖��h��
        if (attackType == ATTACK_SPECIAL_TYPE::JUMP_NOW)
            endTree = true;

        //�A�N�V�����m�[�h���Ȃ��ꍇ��e�A�j���[�V��������
        if (!activeNode_)endTree = true;

        for (int& i : damageAnimAiTreeId_)
        {
            if (endTree)break;

            NodeBase* node = activeNode_.get();
            //��e���ɃA�j���[�V�������邩�m�F
            while (1)
            {
                //���Ȃ�ID�Ȃ�endTree��true��
                if (node->GetId() == i)
                {
                    endTree = true;
                    break;
                }

                //�e���m�F����
                node = node->GetParent();
                if (!node)break;
            }

            //�J�ڂ��I��点�ăA�j���[�V��������
            if (endTree)
            {
                activeNode_->EndActionSetStep();
                activeNode_->Run(this, elapsedTime);
                //����
                activeNode_.release();
            }
        }

        //�_���[�W����
        if (endTree)
        {
            isAnimDamage_ = true;
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();

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
            else if (attackType == ATTACK_SPECIAL_TYPE::JUMP_NOW && isJumpDamage_)    //�󒆔�e��
            {
                animator->SetTriggerOn("damageInAir");
            }
            else
            {
                animator->SetTriggerOn("damage");
            }
        }
    }

    //���ŃW���X�g��𓖂��蔻���؂�A�A�^�b�N�����蔻������Ă���
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
        if(isJustAvoid_)justChild->GetComponent<Collider>()->SetEnabled(true);
        //�C�x���g���Ȃ瓖���蔻����o��
        if (animation->GetCurrentAnimationEvent(animEvent.name.c_str(), pos))
        {
            isJustAvoid_ = false;
            justChild->GetComponent<Collider>()->SetEnabled(true);
        }
    }
}

// GUI�`��
void EnemyCom::OnGUI()
{
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

//��e���ɃA�j���[�V�������鎞��AITREE�����߂�
template<typename... Args>
void EnemyCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
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

//�A�j���[�V�����������ݒ�
void EnemyCom::AnimationInitialize()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //�����̃A�j���[�V����
    animator->SetFirstTransition(IDEL);
    animator->SetLoopAnimation(IDEL, true);

    //�A�j���[�V�����p�����[�^�[�ǉ�
    animator->AddTriggerParameter("kick");
    animator->AddTriggerParameter("rightPunch01");
    animator->AddTriggerParameter("leftUpper01");
    animator->AddTriggerParameter("idle");
    animator->AddTriggerParameter("walk");
    animator->AddTriggerParameter("run");
    animator->AddTriggerParameter("damage");
    animator->AddTriggerParameter("damageInAir");
    animator->AddTriggerParameter("damageGoFly");
    animator->AddTriggerParameter("damageFallEnd");

    animator->AddAnimatorTransition(IDEL);
    animator->SetTriggerTransition(IDEL, "idle");

    animator->AddAnimatorTransition(RUN);
    animator->SetLoopAnimation(RUN, true);
    animator->SetTriggerTransition(RUN, "run");

    animator->AddAnimatorTransition(WALK);
    animator->SetLoopAnimation(WALK, true);
    animator->SetTriggerTransition(WALK, "walk");

    animator->AddAnimatorTransition(KICK);
    animator->SetTriggerTransition(KICK, "kick");

    animator->AddAnimatorTransition(RIGHT_STRAIGHT01, false, 0);
    animator->SetTriggerTransition(RIGHT_STRAIGHT01, "rightPunch01");

    animator->AddAnimatorTransition(LEFT_UPPER01);
    animator->SetTriggerTransition(LEFT_UPPER01, "leftUpper01");

    //��e
    {
        //�m�[�}��
        animator->AddAnimatorTransition(DAMAGE);
        animator->SetTriggerTransition(DAMAGE, "damage");

        //��
        animator->AddAnimatorTransition(DAMAGE_IN_AIR);
        animator->SetTriggerTransition(DAMAGE_IN_AIR, "damageInAir");

        animator->AddAnimatorTransition(DAMAGE_IN_AIR, DAMAGE_FALL, true);
        //animator->SetTriggerTransition(DAMAGE_FALL, DAMAGE_IN_AIR, "damageInAir");

        //�n��[����
        animator->AddAnimatorTransition(DAMAGE_GO_FLY);
        animator->SetTriggerTransition(DAMAGE_GO_FLY, "damageGoFly");


        animator->AddAnimatorTransition(DAMAGE_GO_FLY, DAMAGE_FALL, true);

        animator->SetLoopAnimation(DAMAGE_FALL, true);

        //��e�[�����n
        animator->AddAnimatorTransition(DAMAGE_FALL_END);
        animator->SetTriggerTransition(DAMAGE_FALL_END, "damageFallEnd");

        //�N���オ��
        animator->AddAnimatorTransition(DAMAGE_FALL_END, FALL_STAND_UP, true);
    }


}
