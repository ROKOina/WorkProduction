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

//アニメーションリスト
enum ANIMATION_ENEMY
{
    WALK,
    RUN,
    RUN_BACK,
    JUMP,
    IDEL,
    KICK,
};

//AI遷移
enum class AI_TREE
{
    NONE,
    ROOT,

    //2層
    //ROOT 
    SCOUT,
    BATTLE,

    //3層
    //SCOUT 
    WANDER,
    IDLE,

    //BATTLE
    ATTACK,
    PURSUIT,

    //4層
    //ATTACK
    NORMAL,
};

// 開始処理
void EnemyCom::Start()
{
    //アニメーション初期化
    AnimationInitialize();

    // ビヘイビアツリー設定
    behaviorData_ = std::make_unique<BehaviorData>();
    aiTree_ = std::make_unique<BehaviorTree>(this);
    // BehaviorTree図を基にBehaviorTreeを構築
    aiTree_->AddNode(AI_TREE::NONE, AI_TREE::ROOT, 0, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //2層
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::BATTLE, 1, BehaviorTree::SelectRule::Priority, new BattleJudgment(this), nullptr);
    aiTree_->AddNode(AI_TREE::ROOT, AI_TREE::SCOUT, 2, BehaviorTree::SelectRule::Priority, nullptr, nullptr);

    //3層
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::WANDER, 1, BehaviorTree::SelectRule::Non, new WanderJudgment(this), new WanderAction(this));
    aiTree_->AddNode(AI_TREE::SCOUT, AI_TREE::IDLE, 2, BehaviorTree::SelectRule::Non, nullptr, new IdleAction(this));

    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::ATTACK, 1, BehaviorTree::SelectRule::Random, new AttackJudgment(this), nullptr);
    aiTree_->AddNode(AI_TREE::BATTLE, AI_TREE::PURSUIT, 2, BehaviorTree::SelectRule::Non, nullptr, new PursuitAction(this));

    //4層
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 1, BehaviorTree::SelectRule::Non, new AttackJudgment(this), new AttackAction(this));

    SetRandomTargetPosition();
}

// 更新処理
void EnemyCom::Update(float elapsedTime)
{
    // 現在実行されているノードが無ければ
    if (activeNode_ == nullptr)
    {
        // 次に実行するノードを推論する。
        activeNode_.reset(aiTree_->ActiveNodeInference(behaviorData_.get()));
    }
    // 現在実行するノードがあれば
    if (activeNode_ != nullptr)
    {
         //ビヘイビアツリーからノードを実行。
        NodeBase* n = aiTree_->Run(activeNode_.get(), behaviorData_.get(), elapsedTime);
        if (!n)
        {
            //放棄
            activeNode_.release();
        }
        else
        {
            if (n->GetId() != activeNode_->GetId())
                activeNode_.reset(n);
        }
    }


    //仮でジャスト回避当たり判定を切り、アタック当たり判定をしている
    DirectX::XMFLOAT3 pos;
    //ジャスト
    std::shared_ptr<GameObject> justChild = GetGameObject()->GetChildFind("picolaboAttackJust");
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left_just", pos)) {
        justChild->GetComponent<Collider>()->SetEnabled(true);
        //justChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        justChild->GetComponent<Collider>()->SetEnabled(false);
    }

    //ジャスト当たり判定
    std::vector<HitObj> hitObj = justChild->GetComponent<Collider>()->OnHitGameObject();
    for (auto& h : hitObj)
    {
        if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
        int i = 0;
    }

    //アタックアニメーションイベント取得
    //イベント中は子のアタックオブジェクトをオンに
    std::shared_ptr<GameObject> attackChild = GetGameObject()->GetChildFind("picolaboAttack");
    if (GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationEvent("kick_left", pos)) {
        attackChild->GetComponent<Collider>()->SetEnabled(true);
        attackChild->transform_->SetWorldPosition(pos);
    }
    else
    {
        attackChild->GetComponent<Collider>()->SetEnabled(false);
    }

    ////とりあえず近くにいたら攻撃(仮)
    //DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
    //DirectX::XMFLOAT3 myPos = GetGameObject()->transform_->GetWorldPosition();
    //if (playerPos.x * playerPos.x - myPos.x * myPos.x < 5 * 5)
    //{
    //    if (playerPos.z * playerPos.z - myPos.z * myPos.z < 5 * 5)
    //    {
    //        //アニメーター
    //        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //        animator->SetTriggerOn("kick");
    //    }
    //}

    ////アタック当たり判定
    //hitObj = attackChild->GetComponent<Collider>()->OnHitGameObject();
    //for (auto& h : hitObj)
    //{
    //    if (h.gameObject->GetComponent<Collider>()->GetMyTag() != COLLIDER_TAG::Player)continue;
    //    //プレイヤーにダメージ
    //    std::shared_ptr<GameObject> player = h.gameObject;
    //    if (player->GetComponent<PlayerCom>()->GetIsInvincible())continue;
    //    player->GetComponent<MovementCom>()->AddNonMaxSpeedForce({ 0, 0, -30 });
    //    player->GetComponent<PlayerCom>()->OnDamage();
    //}
}

// GUI描画
void EnemyCom::OnGUI()
{

}

// ターゲット位置をランダム設定
void EnemyCom::SetRandomTargetPosition()
{
    DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
    pos.x += Mathf::RandomRange(-3, 3);
    pos.z += Mathf::RandomRange(-3, 3);
    targetPosition_ = pos;
}

//プレイヤーが近いとtrue
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

//アニメーション初期化設定
void EnemyCom::AnimationInitialize()
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //初期のアニメーション
    animator->SetFirstTransition(IDEL);
    animator->SetLoopAnimation(IDEL, true);

    //アニメーションパラメーター追加
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
