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


// 開始処理
void EnemyCom::Start()
{
    //ダメージアニメーションをするノードを登録
    OnDamageAnimAI_TREE(AI_TREE::WANDER, AI_TREE::PURSUIT/*, AI_TREE::NORMAL*/);
    
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

    //発光を消す
    std::vector<ModelResource::Material>& materials = GetGameObject()->GetComponent<RendererCom>()->GetModel()->GetResourceShared()->GetMaterialsEdit();
    materials[0].toonStruct._Emissive_Color.w = 0;
}

// 更新処理
void EnemyCom::Update(float elapsedTime)
{
    //立ち上がりモーション処理
    StandUpUpdate();

    // 現在実行されているノードが無ければ
    if (activeNode_ == nullptr&& !isAnimDamage_&& !isJumpDamage_&& !isStandUpMotion_)
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

    //重力設定
    GravityProcess(elapsedTime);

    //ダメージ処理
    DamageProcess(elapsedTime);

    //ジャスト回避用判定出す
    justColliderProcess();

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

//被弾時にアニメーションする時のAITREEを決める
template<typename... Args>
void EnemyCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}

//被弾ー＞立ち上がりモーション
void EnemyCom::SetStandUpMotion()
{
    //着地アニメーション入れる
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    animator->SetTriggerOn("damageFallEnd");
    isStandUpMotion_ = true;
    playStandUp_ = false;
    //攻撃を受けても移動しないように
    GetGameObject()->GetComponent<CharacterStatusCom>()->SetAttackNonMove(true);
}
void EnemyCom::StandUpUpdate()
{
    if (isStandUpMotion_)
    {
        std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();
        int index = animation->GetCurrentAnimationIndex();
        if (index == FALL_STAND_UP) //起き上がりモーション時
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

//ダメージ処理
void EnemyCom::DamageProcess(float elapsedTime)
{
    //空中ダメージ時重力を少しの間0にする
    {
        if (!isAnimDamage_ && oldAnimDamage_)
        {
            if (!GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                skyGravityZeroTimer_ = 0.5f;
            }
        }
        oldAnimDamage_ = isAnimDamage_;
    }

    //ダメージアニメーション処理
    if (isAnimDamage_)
    {
        isAnimDamage_ = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageAnimation();
    }

    //ジャンプ被弾時の着地判定
    if (isJumpDamage_)
    {
        if (!isAnimDamage_)
        {
            if (GetGameObject()->GetComponent<MovementCom>()->OnGround())
            {
                isJumpDamage_ = false;
                //立ち上がりモーション起動
                SetStandUpMotion();
            }
        }
    }

    //ダメージ処理
    if (OnDamageEnemy())
    {
        bool endTree = false;   //今の遷移を終わらせるか

        //アンストッパブル被弾ならそのままアニメーション
        ATTACK_SPECIAL_TYPE attackType = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageType();
        if (attackType == ATTACK_SPECIAL_TYPE::UNSTOP)
            endTree = true;
        //ジャンプ中は無防備
        if (attackType == ATTACK_SPECIAL_TYPE::JUMP_NOW)
            endTree = true;

        //アクションノードがない場合被弾アニメーションする
        if (!activeNode_)endTree = true;

        for (int& id : damageAnimAiTreeId_)
        {
            if (endTree)break;

            NodeBase* node = activeNode_.get();
            //被弾時にアニメーションするか確認
            while (1)
            {
                //おなじIDならendTreeをtrueに
                if (node->GetId() == id)
                {
                    endTree = true;
                    break;
                }

                //親も確認する
                node = node->GetParent();
                if (!node)break;
            }

            //遷移を終わらせてアニメーションする
            if (endTree)
            {
                activeNode_->EndActionSetStep();
                activeNode_->Run(this, elapsedTime);
                //放棄
                activeNode_.release();
            }
        }

        //ダメージ処理
        if (endTree)
        {
            isAnimDamage_ = true;
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
            animator->ResetParameterList();

            //アニメーションの種類を判定
            if (isStandUpMotion_)   //立ち上がりモーション中を優先
            {
                SetStandUpMotion();
            }
            else if (attackType == ATTACK_SPECIAL_TYPE::JUMP_START)  //切り上げ被弾時
            {
                animator->SetTriggerOn("damageGoFly");
                isJumpDamage_ = true;

            }
            else if (isJumpDamage_)    //空中被弾時
            {
                animator->SetTriggerOn("damageInAir");
            }
            else
            {
                animator->SetTriggerOn("damage");
            }
        }
    }
}

//ジャスト回避用判定出す
void EnemyCom::justColliderProcess()
{
    //ジャスト回避当たり判定を切り、アタック当たり判定をしている
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
        if (isJustAvoid_)justChild->GetComponent<Collider>()->SetEnabled(true);
        //イベント中なら当たり判定を出す
        if (animation->GetCurrentAnimationEvent(animEvent.name.c_str(), pos))
        {
            isJustAvoid_ = false;
            justChild->GetComponent<Collider>()->SetEnabled(true);
        }
    }
}

//重力設定
void EnemyCom::GravityProcess(float elapsedTime)
{
    std::shared_ptr<AnimationCom> animation = GetGameObject()->GetComponent<AnimationCom>();

    //アニメーションイベントを見て重力をなくす
    if (animation->GetCurrentAnimationEvent("ZeroGravity", DirectX::XMFLOAT3()) || skyGravityZeroTimer_ > 0)
    {
        skyGravityZeroTimer_ -= elapsedTime;
        isSetGravity_ = true;
        GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_ZERO);
        GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
    }
    else
    {
        if (isSetGravity_)   //重力設定をした時だけ入る
        {
            isSetGravity_ = false;
            GetGameObject()->GetComponent<MovementCom>()->SetGravity(GRAVITY_NORMAL);
            GetGameObject()->GetComponent<MovementCom>()->ZeroVelocityY();
        }
    }
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
    animator->AddTriggerParameter("attack");
    animator->AddTriggerParameter("leftUpper01");
    animator->AddTriggerParameter("idle");
    animator->AddTriggerParameter("walk");
    animator->AddTriggerParameter("run");
    animator->AddTriggerParameter("damage");
    animator->AddTriggerParameter("damageInAir");
    animator->AddTriggerParameter("damageGoFly");
    animator->AddTriggerParameter("damageFallEnd");

    animator->AddAnimatorTransition(IDLE_SWORD);
    animator->SetTriggerTransition(IDLE_SWORD, "idle");

    animator->AddAnimatorTransition(RUN_SWORD);
    animator->SetLoopAnimation(RUN_SWORD, true);
    animator->SetTriggerTransition(RUN_SWORD, "run");

    animator->AddAnimatorTransition(WALK_SWORD);
    animator->SetLoopAnimation(WALK_SWORD, true);
    animator->SetTriggerTransition(WALK_SWORD, "walk");

    animator->AddAnimatorTransition(KICK);
    animator->SetTriggerTransition(KICK, "kick");

    animator->AddAnimatorTransition(ATTACK01_SWORD, false, 0);
    animator->SetTriggerTransition(ATTACK01_SWORD, "attack");

    animator->AddAnimatorTransition(LEFT_UPPER01);
    animator->SetTriggerTransition(LEFT_UPPER01, "leftUpper01");

    //被弾
    {
        //ノーマル
        animator->AddAnimatorTransition(DAMAGE);
        animator->SetTriggerTransition(DAMAGE, "damage");

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
    }


}
