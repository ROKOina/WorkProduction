#include "EnemyAppleNearCom.h"
#include "EnemyManager.h"

#include "Components\TransformCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"
#include "../Weapon\WeaponCom.h"
#include "../CharacterStatusCom.h"


#include <imgui.h>

enum APPLE_ANIMATION
{
    IDLE,
    AIR_DAMAGE,
    ATTACK_RUSH,
    FLY_DAMAGE,
    FALL_YARARE,
    WALK,
    FALL_END,
    GET_UP,
};

// �J�n����
void EnemyAppleNearCom::Start()
{
    EnemyManager::Instance().Register(GetGameObject(), EnemyManager::EnemyKind::NEAR_ENEMY);

    //�_���[�W�A�j���[�V����������m�[�h��o�^
    EnemyNearCom::OnDamageAnimAI_TREE(AI_TREE::WANDER, AI_TREE::PURSUIT, AI_TREE::ROUTE, AI_TREE::ATTACK_IDLE/*, AI_TREE::NORMAL*/);

    //�A�j���[�V����������
    AnimationInitialize();
    std::shared_ptr<EnemyCom> myShared = GetGameObject()->GetComponent<EnemyCom>();
    // �r�w�C�r�A�c���[�ݒ�
    behaviorData_ = std::make_unique<BehaviorData>();
    aiTree_ = std::make_unique<BehaviorTree>(myShared);
    // BehaviorTree�}�����BehaviorTree���\�z
    aiTree_->AddNode(AI_TREE::NONE_AI, AI_TREE::ROOT, 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //2�w
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::BATTLE, 1, BehaviorTree::SelectRule::Priority, std::make_shared<BattleJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::SCOUT, 2, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //3�w
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::WANDER, 1, BehaviorTree::SelectRule::Non, std::make_shared<WanderJudgment>(myShared), std::make_shared<WanderAction>(myShared));
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::IDLE, 2, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<IdleAction>(myShared));

    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ATTACK, 1, BehaviorTree::SelectRule::Priority, std::make_shared<AttackJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ROUTE, 2, BehaviorTree::SelectRule::Non, std::make_shared<RoutePathJudgment>(myShared), std::make_shared<RoutePathAction>(myShared));
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::PURSUIT, 3, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<PursuitAction>(myShared));

    //4�w
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::ATTACK_IDLE, 1, BehaviorTree::SelectRule::Non, std::make_shared<AttackIdleJudgment>(myShared), std::make_shared<AttackIdleAction>(myShared));
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 2, BehaviorTree::SelectRule::Non, std::make_shared<AttackJudgment>(myShared), std::make_shared<AttackAction>(myShared));

    SetRandomTargetPosition();

    //�U������X�e�[�^�X
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("attack")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(ATTACK_RUSH, 1, 30, 1.0f, 0.0f);

    //�X�e�[�^�X�ݒ�
    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    status->SetMaxHP(50);
    status->SetHP(50);

    //����������
    std::vector<ModelResource::Material>& materials = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared()->GetMaterialsEdit();
    materials[0].toonStruct._Emissive_Color.w = 0;

    //�N���オ�胂�[�V�����ۑ�
    getUpAnim_ = static_cast<int>(GET_UP);

    //�A�j���[�V�����C�x���gSE������
    animSE.emplace_back("SEDanger", "Data/Audio/Enemy/attackDanger.wav");
    animSE.emplace_back("SEAttack", "Data/Audio/Enemy/appleAttack.wav");
}

// �X�V����
void EnemyAppleNearCom::Update(float elapsedTime)
{
    EnemyNearCom::Update(elapsedTime);
}

// GUI�`��
void EnemyAppleNearCom::OnGUI()
{
    EnemyNearCom::OnGUI();
}

//�A�j���[�V�����������ݒ�
void EnemyAppleNearCom::AnimationInitialize()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //�����̃A�j���[�V����
    animator->SetFirstTransition(IDLE);
    animator->SetLoopAnimation(IDLE, true);

    //�A�j���[�V�����p�����[�^�[�ǉ�
    //enemy���ʃp�����[�^�[
    {
        animator->AddTriggerParameter("attack");
        animator->AddTriggerParameter("damage");
        animator->AddTriggerParameter("damageInAir");
        animator->AddTriggerParameter("damageGoFly");
        animator->AddTriggerParameter("damageFallEnd");

        animator->AddFloatParameter("moveSpeed");
    }

    //idle -> walk
    animator->AddAnimatorTransition(IDLE, WALK);
    animator->SetLoopAnimation(IDLE, true);
    animator->SetFloatTransition(IDLE, WALK,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::GREATER);

    //walk -> idle
    animator->AddAnimatorTransition(WALK, IDLE);
    animator->SetLoopAnimation(WALK, true);
    animator->SetFloatTransition(WALK, IDLE,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::LESS);

    //�U��
    animator->AddAnimatorTransition(ATTACK_RUSH, false, 0);
    animator->SetTriggerTransition(ATTACK_RUSH, "attack");
    animator->AddAnimatorTransition(ATTACK_RUSH, IDLE, true);

    //��e
    {
        //�m�[�}��
        animator->AddAnimatorTransition(AIR_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(AIR_DAMAGE, "damage");
        animator->AddAnimatorTransition(AIR_DAMAGE, IDLE, true);

        //��
        animator->AddAnimatorTransition(AIR_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(AIR_DAMAGE, "damageInAir");

        animator->AddAnimatorTransition(AIR_DAMAGE, FALL_YARARE, true);

        //�n��[����
        animator->AddAnimatorTransition(FLY_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(FLY_DAMAGE, "damageGoFly");


        animator->AddAnimatorTransition(FLY_DAMAGE, FALL_YARARE, true);

        animator->SetLoopAnimation(FALL_YARARE, true);

        //��e�[�����n
        animator->AddAnimatorTransition(FALL_END);
        animator->SetTriggerTransition(FALL_END, "damageFallEnd");

        //�N���オ��
        animator->AddAnimatorTransition(FALL_END, GET_UP, true);

        animator->AddAnimatorTransition(GET_UP, IDLE, true);
    }

}


// ���b�Z�[�W��M�����Ƃ��̏���
bool EnemyAppleNearCom::OnMessage(const Telegram& msg)
{
    return EnemyNearCom::OnMessage(msg);
}
