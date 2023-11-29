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

// 開始処理
void EnemyAppleNearCom::Start()
{
    EnemyManager::Instance().Register(GetGameObject(), EnemyManager::EnemyKind::NEAR_ENEMY);

    //ダメージアニメーションをするノードを登録
    EnemyNearCom::OnDamageAnimAI_TREE(AI_TREE::WANDER, AI_TREE::PURSUIT, AI_TREE::ROUTE, AI_TREE::ATTACK_IDLE/*, AI_TREE::NORMAL*/);

    //アニメーション初期化
    AnimationInitialize();
    std::shared_ptr<EnemyCom> myShared = GetGameObject()->GetComponent<EnemyCom>();
    // ビヘイビアツリー設定
    behaviorData_ = std::make_unique<BehaviorData>();
    aiTree_ = std::make_unique<BehaviorTree>(myShared);
    // BehaviorTree図を基にBehaviorTreeを構築
    aiTree_->AddNode(AI_TREE::NONE_AI, AI_TREE::ROOT, 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //2層
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::BATTLE, 1, BehaviorTree::SelectRule::Priority, std::make_shared<BattleJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::SCOUT, 2, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //3層
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::WANDER, 1, BehaviorTree::SelectRule::Non, std::make_shared<WanderJudgment>(myShared), std::make_shared<WanderAction>(myShared));
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::IDLE, 2, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<IdleAction>(myShared));

    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ATTACK, 1, BehaviorTree::SelectRule::Priority, std::make_shared<AttackJudgment>(myShared), nullptr);
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ROUTE, 2, BehaviorTree::SelectRule::Non, std::make_shared<RoutePathJudgment>(myShared), std::make_shared<RoutePathAction>(myShared));
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::PURSUIT, 3, BehaviorTree::SelectRule::Non, nullptr, std::make_shared<PursuitAction>(myShared));

    //4層
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::ATTACK_IDLE, 1, BehaviorTree::SelectRule::Non, std::make_shared<AttackIdleJudgment>(myShared), std::make_shared<AttackIdleAction>(myShared));
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 2, BehaviorTree::SelectRule::Non, std::make_shared<AttackJudgment>(myShared), std::make_shared<AttackAction>(myShared));

    SetRandomTargetPosition();

    //攻撃判定ステータス
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("attack")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(ATTACK_RUSH, 1, 30, 1.0f, 0.0f);

    //ステータス設定
    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    status->SetMaxHP(50);
    status->SetHP(50);

    //発光を消す
    std::vector<ModelResource::Material>& materials = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared()->GetMaterialsEdit();
    materials[0].toonStruct._Emissive_Color.w = 0;

    //起き上がりモーション保存
    getUpAnim_ = static_cast<int>(GET_UP);

    //アニメーションイベントSE初期化
    animSE.emplace_back("SEDanger", "Data/Audio/Enemy/attackDanger.wav");
    animSE.emplace_back("SEAttack", "Data/Audio/Enemy/appleAttack.wav");
}

// 更新処理
void EnemyAppleNearCom::Update(float elapsedTime)
{
    EnemyNearCom::Update(elapsedTime);
}

// GUI描画
void EnemyAppleNearCom::OnGUI()
{
    EnemyNearCom::OnGUI();
}

//アニメーション初期化設定
void EnemyAppleNearCom::AnimationInitialize()
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //初期のアニメーション
    animator->SetFirstTransition(IDLE);
    animator->SetLoopAnimation(IDLE, true);

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
    animator->AddAnimatorTransition(IDLE, WALK);
    animator->SetLoopAnimation(IDLE, true);
    animator->SetFloatTransition(IDLE, WALK,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::GREATER);

    //walk -> idle
    animator->AddAnimatorTransition(WALK, IDLE);
    animator->SetLoopAnimation(WALK, true);
    animator->SetFloatTransition(WALK, IDLE,
        "moveSpeed", 0.1f, PARAMETER_JUDGE::LESS);

    //攻撃
    animator->AddAnimatorTransition(ATTACK_RUSH, false, 0);
    animator->SetTriggerTransition(ATTACK_RUSH, "attack");
    animator->AddAnimatorTransition(ATTACK_RUSH, IDLE, true);

    //被弾
    {
        //ノーマル
        animator->AddAnimatorTransition(AIR_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(AIR_DAMAGE, "damage");
        animator->AddAnimatorTransition(AIR_DAMAGE, IDLE, true);

        //空中
        animator->AddAnimatorTransition(AIR_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(AIR_DAMAGE, "damageInAir");

        animator->AddAnimatorTransition(AIR_DAMAGE, FALL_YARARE, true);

        //地上ー＞空中
        animator->AddAnimatorTransition(FLY_DAMAGE, false, 0.0f);
        animator->SetTriggerTransition(FLY_DAMAGE, "damageGoFly");


        animator->AddAnimatorTransition(FLY_DAMAGE, FALL_YARARE, true);

        animator->SetLoopAnimation(FALL_YARARE, true);

        //被弾ー＞着地
        animator->AddAnimatorTransition(FALL_END);
        animator->SetTriggerTransition(FALL_END, "damageFallEnd");

        //起き上がり
        animator->AddAnimatorTransition(FALL_END, GET_UP, true);

        animator->AddAnimatorTransition(GET_UP, IDLE, true);
    }

}


// メッセージ受信したときの処理
bool EnemyAppleNearCom::OnMessage(const Telegram& msg)
{
    return EnemyNearCom::OnMessage(msg);
}
