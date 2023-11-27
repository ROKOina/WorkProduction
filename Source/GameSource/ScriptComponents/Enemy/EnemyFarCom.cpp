#include "EnemyFarCom.h"
#include "EnemyManager.h"

#include "Components\TransformCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"

#include "BehaviorTree/JudgmentDerived.h"
#include "BehaviorTree/ActionDerived.h"


#include <imgui.h>

//アニメーションリスト
enum ANIMATION_ENEMY
{
    WALK,
    RUN,
    RUN_BACK,
    JUMP,
    IDEL,
    KICK,
    DAMAGE,
    RIGHT_STRAIGHT01,
    LEFT_UPPER01,
    DAMAGE_FALL,
    DAMAGE_IN_AIR,
    DAMAGE_GO_FLY,
    FALL_STAND_UP,
    DAMAGE_FALL_END,
    ATTACK01_SWORD,
    RUN_SWORD,
    IDLE_SWORD,
    WALK_SWORD,
};

// 開始処理
void EnemyFarCom::Start()
{
    EnemyManager::Instance().Register(GetGameObject(), EnemyManager::EnemyKind::FAR_ENEMY);

    //ダメージアニメーションをするノードを登録
    OnDamageAnimAI_TREE(AI_TREE::WANDER, AI_TREE::PURSUIT/*, AI_TREE::NORMAL*/);

    //アニメーション初期化
    AnimationInitialize();
    // ビヘイビアツリー設定
    behaviorData_ = std::make_unique<BehaviorData>();
    aiTree_ = std::make_unique<BehaviorTree>(GetGameObject()->GetComponent<EnemyCom>());
    // BehaviorTree図を基にBehaviorTreeを構築
    aiTree_->AddNode(AI_TREE::NONE_AI, AI_TREE::ROOT, 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //2層
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::BATTLE, 1, BehaviorTree::SelectRule::Priority, std::make_shared<BattleJudgment>(GetGameObject()->GetComponent<EnemyCom>()), nullptr);
    //aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::SCOUT, 2, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    ////3層
    //aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::WANDER, 1, BehaviorTree::SelectRule::Non, new WanderJudgment(this), new WanderAction(this));
    //aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::IDLE, 2, BehaviorTree::SelectRule::Non, nullptr, new IdleAction(this));

    //aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ATTACK, 1, BehaviorTree::SelectRule::Random, new AttackJudgment(this), nullptr);
    //aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::PURSUIT, 2, BehaviorTree::SelectRule::Non, nullptr, new PursuitAction(this));

    ////4層
    //aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 1, BehaviorTree::SelectRule::Priority, new AttackJudgment(this), new AttackAction(this));

    SetRandomTargetPosition();

    //発光を消す
    std::vector<ModelResource::Material>& materials = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared()->GetMaterialsEdit();
    materials[0].toonStruct._Emissive_Color.w = 0;
}

// 更新処理
void EnemyFarCom::Update(float elapsedTime)
{
    EnemyCom::Update(elapsedTime);
}

// GUI描画
void EnemyFarCom::OnGUI()
{
    EnemyCom::OnGUI();
}

bool EnemyFarCom::OnMessage(const Telegram& msg)
{
    switch (msg.msg)
    {
    case MESSAGE_TYPE::MsgGiveAttackRight:
        isAttackFlag_ = true;
        return true;
    }

    return false;
}

//被弾時にアニメーションする時のAITREEを決める
template<typename... Args>
void EnemyFarCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}


//アニメーション初期化設定
void EnemyFarCom::AnimationInitialize()
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

    //攻撃
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
