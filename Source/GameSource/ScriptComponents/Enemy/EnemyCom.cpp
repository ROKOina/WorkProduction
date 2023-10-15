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
        activeNode_ = aiTree_->ActiveNodeInference(behaviorData_);
    }
    // 現在実行するノードがあれば
    if (activeNode_ != nullptr)
    {
         //ビヘイビアツリーからノードを実行。
        std::shared_ptr<NodeBase> n = aiTree_->Run(activeNode_, behaviorData_, elapsedTime);
        if (!n)
        {
            //放棄
            activeNode_.reset();
        }
        else
        {
            if (n->GetId() != activeNode_->GetId())
                activeNode_.swap(n);
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
    if (ImGui::Button("delete"))
    {
        GameObjectManager::Instance().Remove(GetGameObject());
    }

    ImGui::DragFloat("walkSpeed", &moveDataEnemy.walkSpeed);
    ImGui::DragFloat("walkMaxSpeed", &moveDataEnemy.walkMaxSpeed);
    ImGui::DragFloat("runSpeed", &moveDataEnemy.runSpeed);
    ImGui::DragFloat("runMaxSpeed", &moveDataEnemy.runMaxSpeed);
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
        bool endTree = false;   //今の遷移を終わらせて被弾アニメーションするならtrue

        //アンストッパブル被弾ならそのままアニメーション
        ATTACK_SPECIAL_TYPE attackType = GetGameObject()->GetComponent<CharacterStatusCom>()->GetDamageType();
        if (attackType == ATTACK_SPECIAL_TYPE::UNSTOP)
            endTree = true;
        //ジャンプ中は無防備
        if (attackType == ATTACK_SPECIAL_TYPE::JUMP_NOW)
            endTree = true;

        //アクションノードがない場合
        if (!activeNode_)
            endTree = true;

        //被弾する行動か判断
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
                node = node->GetParent().get();
                if (!node)break;
            }

            //遷移を終わらせてアニメーションする
            if (endTree)
            {
                activeNode_->EndActionSetStep();
                activeNode_->Run(GetGameObject()->GetComponent<EnemyCom>(), elapsedTime);
                //放棄
                activeNode_.reset();
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

bool EnemyCom::OnMessage(const Telegram& msg)
{
    return false;
}





//被弾時にアニメーションする時のAITREEを決める
template<typename... Args>
void EnemyCom::OnDamageAnimAI_TREE(Args... args)
{
    for (AI_TREE i : std::initializer_list<AI_TREE>{ args... }) {
        damageAnimAiTreeId_.emplace_back(static_cast<int>(i));
    }
}
