#include "EnemyCom.h"
#include "Components\AnimationCom.h"
#include "Components\TransformCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimatorCom.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"
#include "GameSource/Math/Mathf.h"

#include "../Player/PlayerCom.h"
#include "../CharacterStatusCom.h"


//AI�J��
enum class AI_TREE
{
    NONE,
    ROOT,

    //2�w
    //ROOT 
    DAMAGE,
    BATTLE,
    SCOUT,

    //3�w
    //SCOUT 
    WANDER,
    IDLE,

    //BATTLE
    ATTACK,
    PURSUIT,

    //4�w
    //ATTACK
    NORMAL,
};

// �J�n����
void EnemyCom::Start()
{
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
}

// �X�V����
void EnemyCom::Update(float elapsedTime)
{
    // ���ݎ��s����Ă���m�[�h���������
    if (activeNode_ == nullptr)
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

    animator->AddAnimatorTransition(DAMAGE);
    animator->SetTriggerTransition(DAMAGE, "damage");

}
