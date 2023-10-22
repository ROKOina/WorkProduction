#include "EnemyNearCom.h"
#include "EnemyManager.h"

#include "Components\TransformCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"


#include <imgui.h>

// 開始処理
void EnemyNearCom::Start()
{
    EnemyManager::Instance().Register(GetGameObject(), EnemyManager::EnemyKind::NEAR_ENEMY);

    //ダメージアニメーションをするノードを登録
    OnDamageAnimAI_TREE(AI_TREE::WANDER, AI_TREE::PURSUIT, AI_TREE::ROUTE/*, AI_TREE::NORMAL*/);

    //アニメーション初期化
    AnimationInitialize();
    std::shared_ptr<EnemyCom> myShared = GetGameObject()->GetComponent<EnemyCom>();
    // ビヘイビアツリー設定
    behaviorData_ = std::make_unique<BehaviorData>();
    aiTree_ = std::make_unique<BehaviorTree>(myShared);
    // BehaviorTree図を基にBehaviorTreeを構築
    aiTree_->AddNode(AI_TREE::NONE, AI_TREE::ROOT, 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //2層
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::BATTLE, 1, BehaviorTree::SelectRule::Priority, std::make_shared<BattleJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::SCOUT, 2, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //3層
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::WANDER, 1, BehaviorTree::SelectRule::Non, std::make_shared<WanderJudgment>(myShared), std::make_shared<WanderAction>(myShared));
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::IDLE, 2, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<IdleAction>(myShared));

    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ATTACK, 1, BehaviorTree::SelectRule::Random, std::make_shared<NearAttackJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ROUTE, 2, BehaviorTree::SelectRule::Non, std::make_shared<RoutePathJudgment>(myShared), std::make_shared<RoutePathAction>(myShared));
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::PURSUIT, 3, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<PursuitAction>(myShared));

    //4層
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 1, BehaviorTree::SelectRule::Priority, std::make_shared<NearAttackJudgment>(myShared), std::make_shared<NearAttackAction>(myShared));

    SetRandomTargetPosition();

    //移動パラメーター設定
    //moveDataEnemy.

    ////発光を消す
    //std::vector<ModelResource::Material>& materials = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared()->GetMaterialsEdit();
    //materials[0].toonStruct._Emissive_Color.w = 0;
}

// 更新処理
void EnemyNearCom::Update(float elapsedTime)
{
    EnemyCom::Update(elapsedTime);

    //接近フラグ管理
    NearFlagProcess();

}

// GUI描画
void EnemyNearCom::OnGUI()
{
    EnemyCom::OnGUI();
}

//被弾時にアニメーションする時のAITREEを決める
template<typename... Args>
void EnemyNearCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}


//アニメーション初期化設定
void EnemyNearCom::AnimationInitialize()
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //初期のアニメーション
    animator->SetFirstTransition(IDLE_SWORD);
    animator->SetLoopAnimation(IDLE_SWORD, true);

    //アニメーションパラメーター追加
    //enemy共通パラメーター
    {
        animator->AddTriggerParameter("attack");
        animator->AddTriggerParameter("damage");
        animator->AddTriggerParameter("damageInAir");
        animator->AddTriggerParameter("damageGoFly");
        animator->AddTriggerParameter("damageFallEnd");

        animator->AddFloatParameter("moveSpeed");
    }

    //idle -> walk
    animator->AddAnimatorTransition(IDLE_SWORD, WALK_SWORD);
    animator->SetLoopAnimation(IDLE_SWORD, true);
    animator->SetFloatTransition(IDLE_SWORD, WALK_SWORD,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::GREATER);

    //walk -> idle
    animator->AddAnimatorTransition(WALK_SWORD, IDLE_SWORD);
    animator->SetLoopAnimation(WALK_SWORD, true);
    animator->SetFloatTransition(WALK_SWORD, IDLE_SWORD,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::LESS);

    //walk -> run
    animator->AddAnimatorTransition(WALK_SWORD, RUN_SWORD);
    animator->SetFloatTransition(WALK_SWORD, RUN_SWORD,
        "moveSpeed", moveDataEnemy_.walkMaxSpeed + 1, PARAMETER_JUDGE::GREATER);

    //run -> walk
    animator->AddAnimatorTransition(RUN_SWORD, WALK_SWORD);
    animator->SetLoopAnimation(RUN_SWORD, true);
    animator->SetFloatTransition(RUN_SWORD, WALK_SWORD,
        "moveSpeed", moveDataEnemy_.walkMaxSpeed + 1, PARAMETER_JUDGE::LESS);


    animator->AddAnimatorTransition(ATTACK01_SWORD, false, 0);
    animator->SetTriggerTransition(ATTACK01_SWORD, "attack");
    animator->AddAnimatorTransition(ATTACK01_SWORD, IDLE_SWORD, true);

    //被弾
    {
        //ノーマル
        animator->AddAnimatorTransition(DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(DAMAGE, "damage");
        animator->AddAnimatorTransition(DAMAGE, IDLE_SWORD, true);

        //空中
        animator->AddAnimatorTransition(DAMAGE_IN_AIR, false, 0.0f);
        animator->SetTriggerTransition(DAMAGE_IN_AIR, "damageInAir");

        animator->AddAnimatorTransition(DAMAGE_IN_AIR, DAMAGE_FALL, true);

        //地上ー＞空中
        animator->AddAnimatorTransition(DAMAGE_GO_FLY, false, 0.0f);
        animator->SetTriggerTransition(DAMAGE_GO_FLY, "damageGoFly");


        animator->AddAnimatorTransition(DAMAGE_GO_FLY, DAMAGE_FALL, true);

        animator->SetLoopAnimation(DAMAGE_FALL, true);

        //被弾ー＞着地
        animator->AddAnimatorTransition(DAMAGE_FALL_END);
        animator->SetTriggerTransition(DAMAGE_FALL_END, "damageFallEnd");

        //起き上がり
        animator->AddAnimatorTransition(DAMAGE_FALL_END, FALL_STAND_UP, true);

        animator->AddAnimatorTransition(FALL_STAND_UP, IDLE_SWORD, true);
    }
}

//接近フラグ管理
void EnemyNearCom::NearFlagProcess()
{
    //接近フラグがonの時離れたらoffにする
    if (!isNearFlag_)return;
    if (activeNode_) 
    {
        //経路探査中は接近フラグを切らない
        if (activeNode_->GetId() == static_cast<int>(AI_TREE::ROUTE))return;
    }

    DirectX::XMVECTOR playerPos = DirectX::XMLoadFloat3(&GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition());
    DirectX::XMVECTOR enemyPos = DirectX::XMLoadFloat3(&GetGameObject()->transform_->GetWorldPosition());
    float nearRadius = EnemyManager::Instance().GetNearEnemyLevel().radius;

    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(playerPos, enemyPos)));

    //接近して無かったら
    if (length > nearRadius)
        isNearFlag_ = false;
}


// メッセージ受信したときの処理
bool EnemyNearCom::OnMessage(const Telegram& msg)
{
    switch (msg.msg)
    {
    case MESSAGE_TYPE::MsgGiveNearRight:
        isNearFlag_ = true;
        return true;
    case MESSAGE_TYPE::MsgGiveAttackRight:
        isAttackFlag_ = true;
        return true;
    }

    return false;
}
