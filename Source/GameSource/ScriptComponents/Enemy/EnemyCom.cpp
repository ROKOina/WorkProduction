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

//�A�j���[�V�������X�g
enum ANIMATION_ENEMY
{
    WALK,
    RUN,
    RUN_BACK,
    JUMP,
    IDEL,
    KICK,
};

//AI�J��
enum class AI_TREE
{
    NONE,
    ROOT,

    //2�w
    //ROOT 
    SCOUT,
    BATTLE,

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
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 1, BehaviorTree::SelectRule::Non, new AttackJudgment(this), new AttackAction(this));

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
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left_just", pos)) {
        justChild->GetComponent<Collider>()->SetEnabled(true);
        //justChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        justChild->GetComponent<Collider>()->SetEnabled(false);
    }

    //�W���X�g�����蔻��
    std::vector<HitObj> hitObj = justChild->GetComponent<Collider>()->OnHitGameObject();
    for (auto& h : hitObj)
    {
        if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
        int i = 0;
    }

    //�A�^�b�N�A�j���[�V�����C�x���g�擾
    //�C�x���g���͎q�̃A�^�b�N�I�u�W�F�N�g���I����
    std::shared_ptr<GameObject> attackChild = GetGameObject()->GetChildFind("picolaboAttack");
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left", pos)) {
        attackChild->GetComponent<Collider>()->SetEnabled(true);
        attackChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        attackChild->GetComponent<Collider>()->SetEnabled(false);
    }

    ////�Ƃ肠�����߂��ɂ�����U��(��)
    //DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
    //DirectX::XMFLOAT3 myPos = GetGameObject()->transform_->GetWorldPosition();
    //if (playerPos.x * playerPos.x - myPos.x * myPos.x < 5 * 5)
    //{
    //    if (playerPos.z * playerPos.z - myPos.z * myPos.z < 5 * 5)
    //    {
    //        //�A�j���[�^�[
    //        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //        animator->SetTriggerOn("kick");
    //    }
    //}

    ////�A�^�b�N�����蔻��
    //hitObj = attackChild->GetComponent<Collider>()->OnHitGameObject();
    //for (auto& h : hitObj)
    //{
    //    if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
    //    //�v���C���[�Ƀ_���[�W
    //    std::shared_ptr<GameObject> player = h.gameObject;
    //    if (player->GetComponent<PlayerCom>()->GetIsInvincible())continue;
    //    player->GetComponent<MovementCom>()->AddNonMaxSpeedForce({ 0, 0, -30 });
    //    player->GetComponent<PlayerCom>()->OnDamage();
    //}
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

    if (dist < 5)
    {
        return true;
    }
    return false;
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
    animator->AddTriggerParameter("idle");
    animator->AddTriggerParameter("walk");
    animator->AddTriggerParameter("run");

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

}
