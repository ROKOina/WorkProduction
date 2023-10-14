#include "EnemyNearCom.h"
#include "EnemyManager.h"

#include "Components\TransformCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"


#include <imgui.h>

// �J�n����
void EnemyNearCom::Start()
{
    EnemyManager::Instance().Register(GetGameObject(), EnemyManager::EnemyKind::NEAR_ENEMY);

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
}

// �X�V����
void EnemyNearCom::Update(float elapsedTime)
{
    EnemyCom::Update(elapsedTime);

    //�ڋ߃t���O�Ǘ�
    NearFlagProcess();

}

// GUI�`��
void EnemyNearCom::OnGUI()
{

}

//��e���ɃA�j���[�V�������鎞��AITREE�����߂�
template<typename... Args>
void EnemyNearCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}


//�A�j���[�V�����������ݒ�
void EnemyNearCom::AnimationInitialize()
{
    //�A�j���[�^�[
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //�����̃A�j���[�V����
    animator->SetFirstTransition(IDLE_SWORD);
    animator->SetLoopAnimation(IDLE_SWORD, true);

    //�A�j���[�V�����p�����[�^�[�ǉ�
    //enemy���ʃg���K�[
    {
        animator->AddTriggerParameter("attack");
        animator->AddTriggerParameter("idle");
        animator->AddTriggerParameter("walk");
        animator->AddTriggerParameter("run");
        animator->AddTriggerParameter("damage");
        animator->AddTriggerParameter("damageInAir");
        animator->AddTriggerParameter("damageGoFly");
        animator->AddTriggerParameter("damageFallEnd");
    }

    animator->AddAnimatorTransition(IDLE_SWORD);
    animator->SetTriggerTransition(IDLE_SWORD, "idle");

    animator->AddAnimatorTransition(RUN_SWORD);
    animator->SetLoopAnimation(RUN_SWORD, true);
    animator->SetTriggerTransition(RUN_SWORD, "run");

    animator->AddAnimatorTransition(WALK_SWORD);
    animator->SetLoopAnimation(WALK_SWORD, true);
    animator->SetTriggerTransition(WALK_SWORD, "walk");

    animator->AddAnimatorTransition(ATTACK01_SWORD, false, 0);
    animator->SetTriggerTransition(ATTACK01_SWORD, "attack");

    //��e
    {
        //�m�[�}��
        animator->AddAnimatorTransition(DAMAGE);
        animator->SetTriggerTransition(DAMAGE, "damage");

        //��
        animator->AddAnimatorTransition(DAMAGE_IN_AIR, false, 0.0f);
        animator->SetTriggerTransition(DAMAGE_IN_AIR, "damageInAir");

        animator->AddAnimatorTransition(DAMAGE_IN_AIR, DAMAGE_FALL, true);

        //�n��[����
        animator->AddAnimatorTransition(DAMAGE_GO_FLY, false, 0.0f);
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

//�ڋ߃t���O�Ǘ�
void EnemyNearCom::NearFlagProcess()
{
    //�ڋ߃t���O��on�̎����ꂽ��off�ɂ���
    if (!isNearFlag_)return;

    DirectX::XMVECTOR playerPos = DirectX::XMLoadFloat3(&GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());
    DirectX::XMVECTOR enemyPos = DirectX::XMLoadFloat3(&GetGameObject()->transform_->GetWorldPosition());
    float nearRadius = EnemyManager::Instance().GetNearEnemyLevel().radius;

    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(playerPos, enemyPos)));

    //�ڋ߂��Ė���������
    if (length > nearRadius)
        isNearFlag_ = false;
}


// ���b�Z�[�W��M�����Ƃ��̏���
bool EnemyNearCom::OnMessage(const Telegram& msg)
{
    switch (msg.msg)
    {
    case MESSAGE_TYPE::MsgGiveNearRight:
        isNearFlag_ = true;
        return true;
    }

    return false;
}