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


//AI遷移
enum class AI_TREE
{
    NONE,
    ROOT,

    //2層
    //ROOT 
    DAMAGE,
    BATTLE,
    SCOUT,

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
    aiTree_->AddNode(AI_TREE::ATTACK, AI_TREE::NORMAL, 1, BehaviorTree::SelectRule::Priority, new AttackJudgment(this), new AttackAction(this));

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

    //ジャスト当たり判定を切っておく
    justChild->GetComponent<Collider>()->SetEnabled(false);

    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
    for (auto& animEvent : animation->GetCurrentAnimationEventsData())
    {
        //justが入っているならなら
        if (animEvent.name.find("just") == std::string::npos)continue;
        if(isJustAvoid_)justChild->GetComponent<Collider>()->SetEnabled(true);
        //イベント中なら当たり判定を出す
        if (animation->GetCurrentAnimationEvent(animEvent.name.c_str(), pos))
        {
            isJustAvoid_ = false;
            justChild->GetComponent<Collider>()->SetEnabled(true);
        }
    }
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

    if (dist < searchRange_)
    {
        return true;
    }
    return false;
}

//ダメージ確認
bool EnemyCom::OnDamageEnemy()
{
    return GetGameObject()->GetComponent<CharacterStatusCom>()->GetFrameDamage();
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
