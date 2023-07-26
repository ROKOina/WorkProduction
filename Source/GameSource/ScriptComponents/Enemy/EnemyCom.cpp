#include "EnemyCom.h"
#include "Components\AnimationCom.h"
#include "Components\TransformCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"
#include "Components\AnimatorCom.h"

#include "../Player/PlayerCom.h"

//アニメーションリスト
enum AnimationEnemy
{
    WALK,
    RUN,
    RUN_BACK,
    JUMP,
    IDEL,
    KICK,
};


// 開始処理
void EnemyCom::Start()
{
    //アニメーション初期化
    AnimationInitialize();
}

// 更新処理
void EnemyCom::Update(float elapsedTime)
{
    //状態
    {
        //ダメージ
        if (isDamage_)
        {
            damageTimer_ += elapsedTime;
            if (damageTimer_ > damageInvincibleTime_)
            {
                isDamage_ = false;
                damageTimer_ = 0;
            }
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

    //とりあえず近くにいたら攻撃(仮)
    DirectX::XMFLOAT3 playerPos = GameObjectManager::Instance().Find("pico")->transform_->GetWorldPosition();
    DirectX::XMFLOAT3 myPos = GetGameObject()->transform_->GetWorldPosition();
    if (playerPos.x * playerPos.x - myPos.x * myPos.x < 5 * 5)
    {
        if (playerPos.z * playerPos.z - myPos.z * myPos.z < 5 * 5)
        {
            //アニメーター
            std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
            animator->SetTriggerOn("kick");
        }
    }

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

//ダメージ
void EnemyCom::OnDamage(DirectX::XMFLOAT3& power)
{
    isDamage_ = true;

    GetGameObject()->GetComponent<MovementCom>()->AddNonMaxSpeedForce(power);
}

//アニメーション初期化設定
void EnemyCom::AnimationInitialize()
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //初期のアニメーション
    animator->SetFirstTransition(AnimationEnemy::IDEL);
    animator->SetLoopAnimation(AnimationEnemy::IDEL, true);

    //アニメーションパラメーター追加
    animator->AddTriggerParameter("kick");

    animator->AddAnimatorTransition(IDEL, KICK);
    animator->SetTriggerTransition(IDEL, KICK, "kick");

    animator->AddAnimatorTransition(KICK, IDEL, true);
}
